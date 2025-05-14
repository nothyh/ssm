#include "pack.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <zip.h>

void log_zip_error(zip_t *archive, const std::string &context_message) {
  const char *err_str = zip_strerror(archive); // 获取与 archive 关联的最新错误
  // 或者使用 zip_get_error 获取更详细的 zip_error_t 结构
  // zip_error_t* err_struct = zip_get_error(archive);
  // const char* err_str = zip_error_strerror(err_struct);
  std::cerr << "libzip Error (" << context_message
            << "): " << (err_str ? err_str : "Unknown error") << std::endl;
}

void log_zip_file_error(zip_file_t *file, const std::string &context_message) {
  const char *err_str =
      zip_file_strerror(file); // 获取与特定 zip_file 关联的错误
  std::cerr << "libzip File Error (" << context_message
            << "): " << (err_str ? err_str : "Unknown error") << std::endl;
}

bool Pack::default_unzip(const fs::path &zip_file_path,
                         const std::string &target_to_extract,
                         const fs::path &dest_path) {
  zip_t *archive = nullptr;
  int zip_err = 0;

  archive = zip_open(zip_file_path.string().c_str(), ZIP_RDONLY, &zip_err);
  if (!archive) {
    zip_error_t error_struct;
    zip_error_init_with_code(&error_struct, zip_err);
    std::cerr << "Error: Cannot open ZIP file '" << zip_file_path
              << "': " << zip_error_strerror(&error_struct)
              << " (code: " << zip_err << ")" << std::endl;
    zip_error_fini(&error_struct);
    return false;
  }

  try {
    if (!fs::exists(dest_path)) {
      fs::create_directories(dest_path);
      std::cout << "Created destination directory: " << dest_path << std::endl;
    } else if (!fs::is_directory(dest_path)) {
      std::cerr << "Error: Destination path '" << dest_path
                << "' exists but is not a directory." << std::endl;
      zip_close(archive);
      return false;
    }
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error creating/accessing destination directory '" << dest_path
              << "': " << e.what() << std::endl;
    zip_close(archive);
    return false;
  }

  // 3. 准备要匹配的前缀 (例如 "Libraries/")
  std::string prefix = target_to_extract;
  // if need unzip a dir , need end with / manually
  if (prefix.empty()) {
    prefix += '/'; // ZIP 内部路径通常用 '/'
  }
  const size_t prefixLen = prefix.length();

  // 4. 获取条目数量
  zip_int64_t num_entries = zip_get_num_entries(archive, 0); // flags = 0
  if (num_entries < 0) {
    log_zip_error(archive, "getting number of entries");
    zip_close(archive);
    return false;
  }
  std::cout << "Archive contains " << num_entries
            << " entries. Searching for prefix '" << prefix << "'..."
            << std::endl;

  bool extracted_something = false;
  const size_t buffer_size = 8192; // 8KB 缓冲区
  std::vector<char> buffer(buffer_size);

  // 5. 遍历所有条目
  for (zip_int64_t i = 0; i < num_entries; ++i) {
    zip_stat_t stat;
    zip_stat_init(&stat); // 初始化结构体

    // 获取条目信息 (名称、大小等)
    if (zip_stat_index(archive, i, 0, &stat) != 0) { // flags = 0
      log_zip_error(archive, "getting stat for index " + std::to_string(i));
      continue; // 跳过这个条目
    }

    // 检查名称是否有效且匹配前缀
    if (!(stat.valid & ZIP_STAT_NAME) || stat.name == nullptr) {
      std::cerr << "Warning: Entry at index " << i << " has no valid name."
                << std::endl;
      continue;
    }

    const char *entryName = stat.name;
    size_t entryNameLen = strlen(entryName);

    if (entryNameLen >= prefixLen &&
        strncmp(entryName, prefix.c_str(), prefixLen) == 0) {
      // 匹配成功！

      // 计算相对路径和完整目标路径
      std::string relativePathStr(entryName, entryNameLen);
      fs::path destEntryPath =
          dest_path / fs::path(relativePathStr).lexically_normal();

      // std::cout << "Processing: " << entryName << " -> " << destEntryPath;
      spdlog::debug("Processing: {} -> {}", std::string(entryName),
                    destEntryPath.string());

      // 检查是目录还是文件 (ZIP 中目录通常以 '/' 结尾)
      bool is_directory = (entryName[entryNameLen - 1] == '/');

      if (is_directory) {
        std::cout << " (Directory)" << std::endl;
        try {
          fs::create_directories(
              destEntryPath); // 创建目录，包括任何需要的父目录
          extracted_something = true;
        } catch (const fs::filesystem_error &e) {
          std::cerr << "\nWarning: Failed to create directory '"
                    << destEntryPath << "': " << e.what() << std::endl;
          // 可以选择继续处理其他文件
        }
      } else {
        std::cout << " (File, size: "
                  << ((stat.valid & ZIP_STAT_SIZE) ? std::to_string(stat.size)
                                                   : "N/A")
                  << ")" << std::endl;

        // 确保父目录存在
        try {
          fs::path parentDir = destEntryPath.parent_path();
          if (!parentDir.empty() && !fs::exists(parentDir)) {
            fs::create_directories(parentDir);
          }
        } catch (const fs::filesystem_error &e) {
          std::cerr << "Error: Failed to create parent directory for '"
                    << destEntryPath << "': " << e.what() << std::endl;
          continue; // 无法创建父目录，跳过此文件
        }

        // 打开 ZIP 中的文件条目进行读取
        zip_file_t *zip_file = zip_fopen_index(archive, i, 0); // flags = 0
        if (!zip_file) {
          log_zip_error(archive,
                        "opening file entry: " + std::string(entryName));
          continue; // 跳过此文件
        }

        // 打开目标文件进行写入 (二进制模式)
        std::ofstream output_file(destEntryPath,
                                  std::ios::binary | std::ios::trunc);
        if (!output_file) {
          std::cerr << "Error: Failed to open output file '" << destEntryPath
                    << "' for writing (errno: " << errno << ")" << std::endl;
          zip_fclose(zip_file); // 关闭 zip 文件句柄
          continue;             // 跳过此文件
        }

        // 循环读取和写入
        zip_int64_t bytes_read;
        bool write_error = false;
        while ((bytes_read = zip_fread(zip_file, buffer.data(), buffer_size)) >
               0) {
          output_file.write(buffer.data(), bytes_read);
          if (!output_file) {
            std::cerr << "\nError: Failed writing to file '" << destEntryPath
                      << "' (errno: " << errno << ")" << std::endl;
            write_error = true;
            break; // 停止写入
          }
        }

        // 检查读取错误
        if (bytes_read < 0) {
          log_zip_file_error(zip_file,
                             "reading file entry: " + std::string(entryName));
          // 文件可能已部分写入，可以考虑在这里删除 output_file
          // fs::remove(destEntryPath);
        }

        // 关闭 ZIP 文件条目句柄 (即使写入失败也要关闭)
        int fclose_ret = zip_fclose(zip_file);
        if (fclose_ret != 0) {
          // zip_file 句柄已无效，无法直接用 log_zip_file_error
          zip_error_t fclose_err_struct;
          zip_error_init_with_code(&fclose_err_struct,
                                   fclose_ret); // 使用返回的错误码
          std::cerr << "Warning: Error closing zip file entry for '"
                    << entryName
                    << "': " << zip_error_strerror(&fclose_err_struct)
                    << std::endl;
          zip_error_fini(&fclose_err_struct);
        }

        // 关闭输出文件流 (ofstream
        // 的析构函数会自动处理，但显式关闭可以立即检查错误状态)
        output_file.close();

        // 只有在没有读写错误且成功关闭文件的情况下，才认为成功提取
        if (!write_error && bytes_read >= 0 && output_file) {
          extracted_something = true;
        } else {
          // 如果有错误，可以选择删除不完整的文件
          try {
            fs::remove(destEntryPath);
            std::cout << "  Removed potentially incomplete file due to error: "
                      << destEntryPath << std::endl;
          } catch (const fs::filesystem_error &rm_err) {
            std::cerr << "  Warning: Could not remove incomplete file '"
                      << destEntryPath << "': " << rm_err.what() << std::endl;
          }
        }
      } // end if(is_directory) else
    } // end if(prefix match)
  } // end for loop

  // 6. 关闭 ZIP 压缩包
  int close_ret = zip_close(archive);
  if (close_ret != 0) {
    // 无法直接获取错误字符串，因为 archive 句柄可能已无效
    // zip_error_t last_error; zip_error_init_with_code(&last_error, close_ret);
    // ? 不确定是否安全
    std::cerr << "Warning: Error closing the main zip archive (code: "
              << close_ret << ")." << std::endl;
    // 即使关闭失败，解压过程可能部分成功了
  }

  if (!extracted_something) {
    std::cout << "No files or directories matching the prefix '" << prefix
              << "' were successfully extracted." << std::endl;
  } else {
    std::cout << "Extraction process finished for folder '" << target_to_extract
              << "'." << std::endl;
  }

  return extracted_something; // 返回是否成功解压了任何内容
}