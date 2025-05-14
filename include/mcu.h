// Define the class to store the mcu specification
#ifndef SSM_INCLUDE_MCU_H
#define SSM_INCLUDE_MCU_H

#include "json.hpp"
#include <filesystem>
#include <iostream>
#include <optional>
#include <set>
#include <string>

namespace fs = std::filesystem;
using json = nlohmann::json;

class MCUConfig {
public:
  std::optional<std::string> full_name;
  std::string series;
  std::string line;
  std::optional<std::set<std::string>> mcu_peripherals;
  friend std::ostream &operator<<(std::ostream &os,
                                  const MCUConfig &mcu_config);
  static MCUConfig from_json(const nlohmann::json &j);
};
class AllConfig {
public:
  MCUConfig mcu_config;
  fs::path std_file_path;
  std::optional<fs::path> keil_file_path;
  fs::path project_path;
  std::set<std::string> user_peripherals;
  friend std::ostream &operator<<(std::ostream &os,
                                  const AllConfig &all_config);
  std::string str() const;
  // construct from a json file
  explicit AllConfig(const fs::path &json_file);
};
#endif
