#include "std_lib.h"
#include "mcu.h"
#include "utils.h"
#include <string>
#include <vector>
StdLib::StdLib(const AllConfig &all_config_ref) : all_config(all_config_ref) {}

void StdLib::unzip() {
  std::vector<std::string> prefixs;
  std::string lib_prefix = "Libraries";
  prefixs.push_back(lib_prefix);
  // libraries
  // config.h
  std::string conf_h_prefix =
      "Project/STM32" + str_to_upper(all_config.mcu_config.series) +
      "xx_StdPeriph_Templates" + "stm32" +
      str_to_lower(all_config.mcu_config.series) + "xx_conf.h";
  prefixs.push_back(conf_h_prefix);
  for (auto const &prefix : prefixs) {
    default_unzip(all_config.std_file_path, prefix, all_config.project_path);
  }
}