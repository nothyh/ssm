#include "mcu.h" // Adjusted path to locate mcu.h
#include "utils.h"
#include <spdlog/spdlog.h>
int main(int argc, char *argv[]) {
  AllConfig all_config;
  parse_args(all_config, argc, argv);
  spdlog::info("Config: {}", all_config.str());
  return 0;
}