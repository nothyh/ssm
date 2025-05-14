#include "keil_pack.h"
#include "mcu.h" // Adjusted path to locate mcu.h
#include "utils.h"
#include <filesystem>
#include <optional>
#include <spdlog/spdlog.h>
#include <stdexcept>
int main(int argc, char *argv[]) {
  std::optional<std::string> config_path_option = parse_args(argc, argv);
  fs::path config_path = fs::path(config_path_option.value_or("./ssm.config"));
  if (!fs::exists(config_path)) {
    throw std::runtime_error("Do not find config file: " +
                             config_path.string());
  }
  AllConfig all_config(config_path);
  spdlog::debug("Config: {}", all_config.str());
  KeilPack keil_pack(all_config);

  return 0;
}