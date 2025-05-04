#include <memory>
#include <filesystem>
#include <iostream>
#include <zip.h>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <regex>
#include <format>

#include "arm_std.h"
#include "utils.h"
#include "logger.h"
namespace fs = std::filesystem;

ArmStd::ArmStd(std::unique_ptr<UserInput> input) : user_input(std::move(input))
{
    // create the unzip_tmp_dir
    fs::path tmp_path = fs::temp_directory_path();
    std::string dir = "tmp_" + std::to_string(std::time(nullptr));
    this->unzip_tmp_dir = tmp_path / dir;
    if (!fs::exists(this->unzip_tmp_dir))
    {
        fs::create_directories(this->unzip_tmp_dir);
        std::cout << "Created temporary directory: " << this->unzip_tmp_dir << std::endl;
    }
    else
    {
        std::cerr << "Temporary directory already exists: " << this->unzip_tmp_dir << std::endl;
    }
    // set the project path
    this->set_project_path();
    this->core_dir = this->project_path / "Core";
    if (!fs::exists(this->core_dir))
    {
        fs::create_directories(this->core_dir);
        std::cout << "Created core directory: " << this->core_dir << std::endl;
    }
    else
    {
        std::cerr << "Core directory already exists: " << this->core_dir << std::endl;
    }
    this->makefile_path = this->project_path / "Makefile";
    this->set_mcu_family();
    this->set_peripherals();
}
ArmStd::~ArmStd()
{
    // Destructor implementation
    // delete the unzip_tmp_dir
    if (fs::exists(this->unzip_tmp_dir))
    {
        fs::remove_all(this->unzip_tmp_dir);
        std::cout << "Deleted temporary directory: " << this->unzip_tmp_dir << std::endl;
    }
    else
    {
        std::cerr << "Temporary directory does not exist: " << this->unzip_tmp_dir << std::endl;
    }
}

// Generalized method to get MCU family, supporting both command-line and GUI
void ArmStd::set_mcu_family()
{
    this->mcu_family = this->user_input->get_mcu_family();

    if (mcu_family.length() < 2)
    {
        std::cerr << "Error: MCU family name is too short. Try again!" << std::endl;
        this->mcu_family = this->user_input->get_mcu_family();
    }
    else
    {
        for (auto &s : mcu_family)
        {
            std::toupper(s);
        }
    }
}
void ArmStd::set_peripherals()
{
    this->user_peripherals = this->user_input->get_peripherals();
    this->user_peripherals.insert("rcc");
}

void ArmStd::set_project_path()
{
    this->project_path = fs::path{this->user_input->get_project_path()};
    if (this->project_path.is_relative())
    {
        this->project_path = fs::current_path() / this->project_path;
        this->project_path = fs::absolute(this->project_path);
    }
    if (!fs::exists(this->project_path))
    {
        std::cerr << "Error: Project path does not exist, Try to create!" << std::endl;
        if (!fs::create_directories(this->project_path))
        {
            std::cerr << "Error: Failed to create project path!" << std::endl;
        }
        else
        {
            std::cout << "Project path: " << fs::absolute(this->project_path) << "has been created" << std::endl;
        }
    }
}

void log_zip_error(zip_t *archive, const std::string &context_message)
{
    const char *err_str = zip_strerror(archive); // 获取与 archive 关联的最新错误
    // 或者使用 zip_get_error 获取更详细的 zip_error_t 结构
    // zip_error_t* err_struct = zip_get_error(archive);
    // const char* err_str = zip_error_strerror(err_struct);
    std::cerr << "libzip Error (" << context_message << "): " << (err_str ? err_str : "Unknown error") << std::endl;
}

void log_zip_file_error(zip_file_t *file, const std::string &context_message)
{
    const char *err_str = zip_file_strerror(file); // 获取与特定 zip_file 关联的错误
    std::cerr << "libzip File Error (" << context_message << "): " << (err_str ? err_str : "Unknown error") << std::endl;
}

bool ArmStd::extract_lib(const fs::path &zip_file_path, const std::string &target_to_extract, const fs::path &dest_path)
{
    zip_t *archive = nullptr;
    int zip_err = 0;

    archive = zip_open(zip_file_path.string().c_str(), ZIP_RDONLY, &zip_err);
    if (!archive)
    {
        zip_error_t error_struct;
        zip_error_init_with_code(&error_struct, zip_err);
        std::cerr << "Error: Cannot open ZIP file '" << zip_file_path << "': "
                  << zip_error_strerror(&error_struct) << " (code: " << zip_err << ")" << std::endl;
        zip_error_fini(&error_struct);
        return false;
    }

    try
    {
        if (!fs::exists(dest_path))
        {
            fs::create_directories(dest_path);
            std::cout << "Created destination directory: " << dest_path << std::endl;
        }
        else if (!fs::is_directory(dest_path))
        {
            std::cerr << "Error: Destination path '" << dest_path << "' exists but is not a directory." << std::endl;
            zip_close(archive);
            return false;
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error creating/accessing destination directory '" << dest_path << "': " << e.what() << std::endl;
        zip_close(archive);
        return false;
    }

    // 3. 准备要匹配的前缀 (例如 "Libraries/")
    std::string prefix = target_to_extract;
    if (!prefix.empty() && prefix.back() != '/')
    {
        prefix += '/'; // ZIP 内部路径通常用 '/'
    }
    const size_t prefixLen = prefix.length();
    // does not provide prefix, which means extract all
    if (prefix.empty())
    {
    }

    // 4. 获取条目数量
    zip_int64_t num_entries = zip_get_num_entries(archive, 0); // flags = 0
    if (num_entries < 0)
    {
        log_zip_error(archive, "getting number of entries");
        zip_close(archive);
        return false;
    }
    std::cout << "Archive contains " << num_entries << " entries. Searching for prefix '" << prefix << "'..." << std::endl;

    bool extracted_something = false;
    const size_t buffer_size = 8192; // 8KB 缓冲区
    std::vector<char> buffer(buffer_size);

    // 5. 遍历所有条目
    for (zip_int64_t i = 0; i < num_entries; ++i)
    {
        zip_stat_t stat;
        zip_stat_init(&stat); // 初始化结构体

        // 获取条目信息 (名称、大小等)
        if (zip_stat_index(archive, i, 0, &stat) != 0)
        { // flags = 0
            log_zip_error(archive, "getting stat for index " + std::to_string(i));
            continue; // 跳过这个条目
        }

        // 检查名称是否有效且匹配前缀
        if (!(stat.valid & ZIP_STAT_NAME) || stat.name == nullptr)
        {
            std::cerr << "Warning: Entry at index " << i << " has no valid name." << std::endl;
            continue;
        }

        const char *entryName = stat.name;
        size_t entryNameLen = strlen(entryName);

        if (entryNameLen > prefixLen && strncmp(entryName, prefix.c_str(), prefixLen) == 0)
        {
            // 匹配成功！

            // 计算相对路径和完整目标路径
            std::string relativePathStr = entryName + prefixLen;
            fs::path destEntryPath = dest_path / fs::path(relativePathStr).lexically_normal();

            std::cout << "Processing: " << entryName << " -> " << destEntryPath;

            // 检查是目录还是文件 (ZIP 中目录通常以 '/' 结尾)
            bool is_directory = (entryName[entryNameLen - 1] == '/');

            if (is_directory)
            {
                std::cout << " (Directory)" << std::endl;
                try
                {
                    fs::create_directories(destEntryPath); // 创建目录，包括任何需要的父目录
                    extracted_something = true;
                }
                catch (const fs::filesystem_error &e)
                {
                    std::cerr << "\nWarning: Failed to create directory '" << destEntryPath << "': " << e.what() << std::endl;
                    // 可以选择继续处理其他文件
                }
            }
            else
            {
                std::cout << " (File, size: " << ((stat.valid & ZIP_STAT_SIZE) ? std::to_string(stat.size) : "N/A") << ")" << std::endl;

                // 确保父目录存在
                try
                {
                    fs::path parentDir = destEntryPath.parent_path();
                    if (!parentDir.empty() && !fs::exists(parentDir))
                    {
                        fs::create_directories(parentDir);
                    }
                }
                catch (const fs::filesystem_error &e)
                {
                    std::cerr << "Error: Failed to create parent directory for '" << destEntryPath << "': " << e.what() << std::endl;
                    continue; // 无法创建父目录，跳过此文件
                }

                // 打开 ZIP 中的文件条目进行读取
                zip_file_t *zip_file = zip_fopen_index(archive, i, 0); // flags = 0
                if (!zip_file)
                {
                    log_zip_error(archive, "opening file entry: " + std::string(entryName));
                    continue; // 跳过此文件
                }

                // 打开目标文件进行写入 (二进制模式)
                std::ofstream output_file(destEntryPath, std::ios::binary | std::ios::trunc);
                if (!output_file)
                {
                    std::cerr << "Error: Failed to open output file '" << destEntryPath << "' for writing (errno: " << errno << ")" << std::endl;
                    zip_fclose(zip_file); // 关闭 zip 文件句柄
                    continue;             // 跳过此文件
                }

                // 循环读取和写入
                zip_int64_t bytes_read;
                bool write_error = false;
                while ((bytes_read = zip_fread(zip_file, buffer.data(), buffer_size)) > 0)
                {
                    output_file.write(buffer.data(), bytes_read);
                    if (!output_file)
                    {
                        std::cerr << "\nError: Failed writing to file '" << destEntryPath << "' (errno: " << errno << ")" << std::endl;
                        write_error = true;
                        break; // 停止写入
                    }
                }

                // 检查读取错误
                if (bytes_read < 0)
                {
                    log_zip_file_error(zip_file, "reading file entry: " + std::string(entryName));
                    // 文件可能已部分写入，可以考虑在这里删除 output_file
                    // fs::remove(destEntryPath);
                }

                // 关闭 ZIP 文件条目句柄 (即使写入失败也要关闭)
                int fclose_ret = zip_fclose(zip_file);
                if (fclose_ret != 0)
                {
                    // zip_file 句柄已无效，无法直接用 log_zip_file_error
                    zip_error_t fclose_err_struct;
                    zip_error_init_with_code(&fclose_err_struct, fclose_ret); // 使用返回的错误码
                    std::cerr << "Warning: Error closing zip file entry for '" << entryName << "': "
                              << zip_error_strerror(&fclose_err_struct) << std::endl;
                    zip_error_fini(&fclose_err_struct);
                }

                // 关闭输出文件流 (ofstream 的析构函数会自动处理，但显式关闭可以立即检查错误状态)
                output_file.close();

                // 只有在没有读写错误且成功关闭文件的情况下，才认为成功提取
                if (!write_error && bytes_read >= 0 && output_file)
                {
                    extracted_something = true;
                }
                else
                {
                    // 如果有错误，可以选择删除不完整的文件
                    try
                    {
                        fs::remove(destEntryPath);
                        std::cout << "  Removed potentially incomplete file due to error: " << destEntryPath << std::endl;
                    }
                    catch (const fs::filesystem_error &rm_err)
                    {
                        std::cerr << "  Warning: Could not remove incomplete file '" << destEntryPath << "': " << rm_err.what() << std::endl;
                    }
                }
            } // end if(is_directory) else
        } // end if(prefix match)
    } // end for loop

    // 6. 关闭 ZIP 压缩包
    int close_ret = zip_close(archive);
    if (close_ret != 0)
    {
        // 无法直接获取错误字符串，因为 archive 句柄可能已无效
        // zip_error_t last_error; zip_error_init_with_code(&last_error, close_ret); ? 不确定是否安全
        std::cerr << "Warning: Error closing the main zip archive (code: " << close_ret << ")." << std::endl;
        // 即使关闭失败，解压过程可能部分成功了
    }

    if (!extracted_something)
    {
        std::cout << "No files or directories matching the prefix '" << prefix << "' were successfully extracted." << std::endl;
    }
    else
    {
        std::cout << "Extraction process finished for folder '" << target_to_extract << "'." << std::endl;
    }

    return extracted_something; // 返回是否成功解压了任何内容
}

bool ArmStd::extrect_lib_to_tmp(const fs::path &zip_file_path)
{
    bool ret = this->extract_lib(zip_file_path, "", this->unzip_tmp_dir);
    return ret;
}

bool ArmStd::get_from_lib_to_proj(const std::string &prefix_pattern)
{
    get_from_lib_to(prefix_pattern, this->project_path);
    return true;
}
bool ArmStd::get_from_lib_to(const std::string &prefix, const std::string &dest_path)
{
    // prefix pattern
    std::regex prefix_re = std::regex(prefix);
    // iterate the entry and find match
    fs::path target;
    for (auto const &entry : fs::recursive_directory_iterator(this->unzip_tmp_dir))
    {
        if (std::regex_match(entry.path().string(), prefix_re))
        {
            target = entry.path();
            break;
        }
        // copy the file to dest
    }

    if (target.empty())
    {
        std::cerr << "Error: No matching file found in the temporary directory." << std::endl;
        return false;
    }

    // split the path by /
    if (!fs::exists(target))
    {
        std::cerr << "Error: Target path does not exist: " << target << std::endl;
        return false;
    }
    fs::path dest = fs::path{dest_path} / target.filename();

    if (fs::is_directory(target))
    {
        fs::copy(target, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        std::cout << "Copied directory from " << target << " to " << dest << std::endl;
    }
    else if (fs::is_regular_file(target))
    {
        fs::copy_file(target, dest, fs::copy_options::overwrite_existing);
        std::cout << "Copied file from " << target << " to " << dest << std::endl;
    }
    else
    {
        std::cerr << "Error: Target is neither a file nor a directory: " << target << std::endl;
        return false;
    }
    return true;
}
bool ArmStd::construct_core_dir()

{
    fs::path inc = core_dir / "Inc";
    fs::path src = core_dir / "Src";
    if (!fs::exists(inc))
    {
        fs::create_directories(inc);
        std::cout << "Created Inc directory: " << inc << std::endl;
    }
    else
    {
        std::cerr << "Inc directory already exists: " << inc << std::endl;
        return false;
    }
    if (!fs::exists(src))
    {
        fs::create_directories(src);
        std::cout << "Created Src directory: " << src << std::endl;
    }
    else
    {
        std::cerr << "Src directory already exists: " << src << std::endl;
        return false;
    }
    // use mcu_family to construct a prefix pattern
    std::string it_h = R"(.*/STM32.*/Project/.*Template/stm32)" + str_to_lower(mcu_family) + R"(x_it.h)";
    std::string conf_h = R"(.*/STM32.*/Project/.*Template/stm32)" + str_to_lower(mcu_family) + R"(x_conf.h)";
    std::string it_c = R"(.*/STM32.*/Project/.*Template/stm32)" + str_to_lower(mcu_family) + R"(x_it.c)";
    // std::string it_c = R"(.*/STM32.*/Project/.*Template/stm32.*\.c)";
    get_from_lib_to(it_h, inc.string());
    get_from_lib_to(conf_h, inc.string());
    get_from_lib_to(it_c, src.string());
    copy_from_to(fs::path{"/home/hyh/workspace/proj/ssm/ssm/resources/main.c"}, fs::path{src / "main.c"});
    copy_from_to(fs::path{"/home/hyh/workspace/proj/ssm/ssm/resources/STM32F103XX_FLASH.ld"}, fs::path{project_path / "STM32F103XX_FLASH.ld"});

    return false;
}

bool ArmStd::copy_from_to(const fs::path &src, const fs::path &dest)
{
    if (!fs::exists(src))
    {
        std::cerr << "Error: Source path does not exist: " << src << std::endl;
        return false;
    }
    if (fs::is_directory(src))
    {
        fs::copy(src, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        std::cout << "Copied directory from " << src << " to " << dest << std::endl;
    }
    else if (fs::is_regular_file(src))
    {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
        std::cout << "Copied file from " << src << " to " << dest << std::endl;
    }
    else
    {
        std::cerr << "Error: Source is neither a file nor a directory: " << src << std::endl;
        return false;
    }
    return true;
}

fs::path ArmStd::get_unzip_gmp_dir()
{
    return this->unzip_tmp_dir;
}

void ArmStd::construct_makefile()
{
    create_makefile();
    modify_makefile();
}

void ArmStd::modify_makefile()
{
    // read
    std::ifstream makefile(this->makefile_path, std::ios::binary | std::ios::ate);
    if (!makefile)
    {
        throw std::system_error(errno, std::generic_category(), "Failed to open " + this->makefile_path.string());
    }
    const auto file_size = makefile.tellg();
    if (file_size == std::ifstream::pos_type(-1))
    {
        throw std::system_error(errno, std::generic_category(), "Failed to get size of " + this->makefile_path.string());
    }
    // create a string to save the content
    std::string content(file_size, '\0');
    makefile.seekg(0);
    if (!makefile.read(&content[0], file_size))
    {
        throw std::system_error(errno, std::generic_category(), "Failed to read " + this->makefile_path.string());
    }
    // close the file
    makefile.close();
    // replace the content
    std::vector<std::pair<std::string, std::string>> replacements{
        {"{{TARGET}}", this->project_path.filename().string()},
        {"{{C_SOURCES}}", get_c_sources()},
        {"{{ASM_SOURCES}}", get_asm_sources()},
        {"{{C_DEFS}}", get_c_defs()},
        {"{{C_INCLUDES}}", get_c_includes()},
        {"{{CPU}}", get_cpu()},
        {"{{FPU}}", get_fpu()},
        {"{{FLOAT-ABI}}", get_float_abi()},
        {"{{LDSCRIPT}}", get_ld_script()},

    };

    for (size_t i = 0; i < replacements.size(); ++i)
    {
        string_replace(content, replacements[i].first, replacements[i].second);
    }
    const fs::path makefile_tmp = makefile_path.parent_path() / "Makefile.tmp";
    {
        std::ofstream out{makefile_tmp, std::ios::binary};
        out << content;
    }
    fs::rename(makefile_tmp, makefile_path);
}

void ArmStd::create_makefile()
{
    fs::path makefile_template{"/home/hyh/workspace/proj/ssm/ssm/resources/Makefile.template"};
    try
    {
        fs::copy(makefile_template, makefile_path, fs::copy_options::skip_existing);
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error copying Makefile template: " << e.what() << std::endl;
        return;
    }

    if (!fs::exists(makefile_path))
    {
        std::cerr << "Error: Makefile already exists: " << makefile_path << std::endl;
    }
}

std::string ArmStd::peripherals_to_file()
{
    std::string peripherals;
    std::string one;
    fs::path one_path;
    for (auto &peripheral : this->user_peripherals)
    {
        one = std::string("Libraries/") + "STM32" + str_to_upper(this->mcu_family) + "x_" + "StdPeriph_Driver/src/stm32" + this->mcu_family + "x_" + peripheral + ".c";
        one_path = this->project_path / one;
        if (!fs::exists(one_path))
        {
            std::cerr << "Error: Peripheral file does not exist: " << one_path << std::endl;
            continue;
        }
        peripherals.append(one + " \\" + "\n");
    }
    return peripherals;
}

std::string ArmStd::get_c_sources()
{
    std::string c_sources;
    c_sources.append("Core/Src/main.c");
    c_sources.append(" \\\n");
    c_sources.append(std::format("Core/Src/stm32{}x_it.c", this->mcu_family));
    c_sources.append(" \\\n");
    c_sources.append(std::format("Libraries/CMSIS/CM3/DeviceSupport/ST/STM32{}x/system_stm32{}x.c", str_to_upper(mcu_family), str_to_lower(mcu_family)));
    c_sources.append(" \\\n");
    c_sources.append(peripherals_to_file());
    return c_sources;
}
std::string ArmStd::get_asm_sources()
{
    // TODO CM3
    std::string asm_sources = std::format("Libraries/CMSIS/CM3/DeviceSupport/ST/STM32{}x/startup/gcc_ride7/startup_stm32{}x_md.s", str_to_upper(mcu_family), str_to_lower(mcu_family));
    return asm_sources;
}
std::string ArmStd::get_c_includes()
{
    // TODO CM3
    std::string c_includes;
    c_includes.append(std::format("-ILibraries/CMSIS/CM3/DeviceSupport/ST/STM32{}x", str_to_upper(mcu_family)));
    c_includes.append(" \\\n");
    c_includes.append("-ILibraries/CMSIS/CM3/CoreSupport");
    c_includes.append(" \\\n");
    c_includes.append(std::format("-ILibraries/STM32{}x_StdPeriph_Driver/inc", str_to_upper(mcu_family)));
    c_includes.append(" \\\n");
    c_includes.append("-ICore/Inc");
    return c_includes;
}

std::string ArmStd::get_c_defs()
{
    std::string c_defs;
    c_defs.append(std::format("-DSTM32{}X_MD \\\n", str_to_upper(mcu_family)));
    c_defs.append("-DUSE_STDPERIPH_DRIVER \\\n");
    return c_defs;
}
std::string ArmStd::get_cpu()
{
    std::string cpu{"-mcpu=cortex-m3"};
    return cpu;
}
std::string ArmStd::get_fpu()
{
    return "";
}
std::string ArmStd::get_float_abi()
{
    return "";
}
std::string ArmStd::get_ld_script()
{
    std::string ld_script = std::format("STM32{}3XX_FLASH.ld", str_to_upper(mcu_family));
    return ld_script;
}