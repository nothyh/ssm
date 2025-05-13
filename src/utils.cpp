#include "utils.h"
#include "mcu.h"
#include "spdlog/spdlog.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <spdlog/common.h>
void ensure_value(int argc, char *argv[], int i) {
  // ssm -s f1
  // argc = 3, option: i = 1, value: i+1 = 2
  int value_index = i + 1;
  if (argc <= value_index) {
    spdlog::info("need value for option");
    exit(EXIT_FAILURE);
  }
  if (argv[value_index][0] == '-') {
    spdlog::info("Invalid value of option");
    exit(EXIT_FAILURE);
  }
}
void parse_args(AllConfig &all_config, int argc, char *argv[]) {
  if (argc < 2) {
    print_usage();
    exit(0);
  }
  std::string arg;
  for (int i = 0; i < argc; i++) {
    arg = argv[i];
    if (arg == "-s" || arg == "--series") {
      ensure_value(argc, argv, i);
      all_config.mcu_config.series = argv[++i];
    } else if (arg == "-l" || arg == "--line") {
      ensure_value(argc, argv, i);
      all_config.mcu_config.line = argv[++i];
    } else if (arg == "-sp" || arg == "--stdpath") {
      ensure_value(argc, argv, i);
      all_config.std_file_path = fs::path(argv[++i]);
    } else if (arg == "-p" || arg == "--pro") {
      ensure_value(argc, argv, i);
      all_config.project_path = fs::path(argv[++i]);
    } else if (arg == "-pe" || arg == "--peri") {
      // if (i != (argc - 1)) {
      //  spdlog::info("put peripherlals at the end of command");
      //  print_usage();
      //  exit(EXIT_FAILURE);
      //}

      // ssm -pe gpio tim -s f1
      // argc = 6, i = 1
      ensure_value(argc, argv, i);
      // j = 2, insert
      // j = 3, insert
      // j = 4, break
      int value_index = ++i;
      for (; value_index < argc; value_index++) {
        if (argv[value_index][0] != '-') {
          all_config.user_peripherals.insert(argv[value_index]);
        }
      }
      // j = 4
      i = value_index - 1;
      // i = 3
    }

    else if (arg == "-d" || arg == "--debug") {
      spdlog::set_level(spdlog::level::debug);
    } else if (arg == "-q" || arg == "--quiet") {
      spdlog::set_level(spdlog::level::off);
    }
  }
}

void print_usage() { std::cout << "hello"; }