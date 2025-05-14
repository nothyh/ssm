#include "keil_pack.h"
#include "mcu.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <curl/curl.h>
#include <filesystem>
#include <iostream>
#include <pugixml.hpp>
#include <spdlog/spdlog.h>
#include <string>
KeilPack::~KeilPack() {}

KeilPack::KeilPack(const AllConfig &all_config_src)
    : all_config(all_config_src),
      short_name("STM32" + str_to_upper(all_config.mcu_config.series) +
                 "xx_DFP") {
  // if keil pack exist, do not download
  // unzip and parse
  // keil_pack:
  // TODO: for windows
  ssm_path = fs::path(std::getenv("HOME")) / "ssm";
  if (!fs::exists(ssm_path)) {
    fs::create_directory(ssm_path);
    SPDLOG_DEBUG("create ssm dir first time");
  }

  fs::path tmp_path = fs::temp_directory_path();
  std::string dir = "tmp_" + std::to_string(std::time(nullptr));
  unzip_path = tmp_path / dir;
  // 如果提供了pack路径，不下载，直接解压，然后解析
  if (!find_existing_pack()) {
    // download
    set_full_pack_url(download_index_file_to_string());
    download_pack(fs::path(ssm_path / pack_name));
  }
  unzip();

  // 否则，到~/ssm下寻找, 如果找到 解压，解析
  // 否则下载到指定路径或~/ssm 或当前文件夹, 解压，解析
  //
  // 解压用
  // 解压：解压到tmp下随机
  // 解析：解析后，参数传递给all_config
}

bool KeilPack::find_existing_pack() {
  if (all_config.keil_file_path.has_value() &&
      fs::exists(all_config.keil_file_path.value())) {
    pack_path = all_config.keil_file_path.value();
    return true;
  } else {
    std::string name =
        "STM32" + str_to_upper(all_config.mcu_config.series) + "xx_DFP";
    for (auto const &dir_entry : fs::directory_iterator{ssm_path}) {
      if (dir_entry.path().filename().string().find(name) !=
          std::string::npos) {
        pack_path = dir_entry.path();
        spdlog::debug("Find keil pack at {}", pack_path.string());
        return true;
      }
    }
  }
  return false;
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  std::string *str = (std::string *)userp;
  str->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string KeilPack::download_index_file_to_string() {
  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize CURL" << std::endl;
    return "";
  }
  std::string readBuffer;
  curl_easy_setopt(curl, CURLOPT_URL, PACK_INDEX_FILE_URL.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    curl_easy_cleanup(curl);
    return "";
  }
  curl_easy_cleanup(curl);
  return readBuffer;
}

void KeilPack::set_full_pack_url(const std::string &index_file_in_xml) {
  pugi::xml_document doc;
  pugi::xml_parse_result parse_result =
      doc.load_string(index_file_in_xml.c_str());
  if (!parse_result) {
    std::cerr << "Failed to parse XML: " << parse_result.description()
              << std::endl;
    return;
  }
  std::string xPath = "//pindex/pdsc[@name='" + this->short_name + "']";
  pugi::xpath_node_set nodes = doc.select_nodes(xPath.c_str());
  if (nodes.empty()) {
    std::cerr << "No nodes found for XPath: " << xPath << std::endl;
    return;
  }
  if (nodes.size() > 1) {
    std::cerr << "Multiple nodes found for XPath: " << xPath << std::endl;
    return;
  }
  pugi::xml_node node = nodes.first().node();
  xml_entry.url = node.attribute("url").as_string();
  xml_entry.version = node.attribute("version").as_string();
  xml_entry.vendor = node.attribute("vendor").as_string();
  pack_name =
      xml_entry.vendor + "." + short_name + "." + xml_entry.version + ".pack";
  this->full_url = xml_entry.url + pack_name;
}
void KeilPack::download_pack(const std::string &dest) {
  CURL *curl = curl_easy_init();
  FILE *fp = fopen(dest.c_str(), "wb+");
  if (!fp) {
    std::cerr << "Failed to open file: " << dest << std::endl;
    return;
  }

  if (!curl) {
    std::cerr << "Failed to initialize CURL" << std::endl;
    return;
  }
  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    fclose(fp);
    curl_easy_cleanup(curl);
    return;
  }
  curl_easy_cleanup(curl);
  fclose(fp);
}

void KeilPack::unzip() {
  // Keil.STM32F4xx_DFP.pdsc
  std::string prefix = "Keil." + short_name + ".pdsc";
  default_unzip(pack_path, prefix, unzip_path);
}