// Define the inner structure of mcu configuration
#include "mcu.h"
#include <ostream>
#include <sstream>
std::ostream &operator<<(std::ostream &os, const AllConfig &config) {
  os << "AllConfig:\n";
  os << "  config_path: " << config.config_path << "\n";
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
  for (const auto &p : config.mcu_peripherals)
    os << p << " ";
  return os;
}

std::string AllConfig::str() const {
  std::ostringstream oss;
  oss << *this;
  return oss.str();
}