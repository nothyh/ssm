// Define the inner structure of mcu configuration
#include "mcu.h"
#include <filesystem>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdexcept>
namespace fs = std::filesystem;
std::ostream &operator<<(std::ostream &os, const AllConfig &config) {
  os << "AllConfig:\n";
  os << "  mcu_config: " << config.mcu_config << "\n"; // 复用MCUConfig的输出
  os << "  std_file_path: " << config.std_file_path << "\n";
  os << "  project_path: " << config.project_path << "\n";
  os << "  user_peripherals: ";
  for (const auto &p : config.user_peripherals)
    os << p << " ";
  return os;
}

std::ostream &operator<<(std::ostream &os, const MCUConfig &config) {
  os << "MCUConfig:\n";
  os << "  full_name: " << (config.full_name ? *config.full_name : "nullopt")
     << "\n";
  os << "  series: " << config.series << "\n";
  os << "  line: " << config.line << "\n";
  os << "  peripherals: ";
  return os;
}

std::string AllConfig::str() const {
  std::ostringstream oss;
  oss << *this;
  return oss.str();
}
AllConfig::AllConfig(const fs::path &json_file_path) {
  if (!fs::exists(json_file_path)) {
    throw std::runtime_error("Does not find json file: " +
                             json_file_path.string());
  }
  std::ifstream json_file(json_file_path);
  if (!json_file.is_open()) {
    throw std::runtime_error("Failed to open JSON config file:" +
                             json_file_path.string());
  }
  nlohmann::json config_json;
  json_file >> config_json;
  std_file_path = fs::path(config_json.value("std_file_path", ""));
  project_path = fs::path(config_json.value("project_path", ""));
  if (config_json.contains("user_peripherals")) {
    for (const auto &per : config_json["user_peripherals"]) {
      user_peripherals.insert(per.get<std::string>());
    }
  }
  if (config_json.contains("mcu_config")) {
    mcu_config = MCUConfig::from_json(config_json["mcu_config"]);
  }
}
MCUConfig MCUConfig::from_json(const nlohmann::json &j) {
  MCUConfig mcu_config;
  mcu_config.full_name = j.value("full_name", "");
  mcu_config.series = j.value("series", "");
  mcu_config.line = j.value("line", "");
  return mcu_config;
}
