#include "utils.h"
#include "spdlog/spdlog.h"
#include <cstdlib>
#include <iostream>
#include <optional>
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
std::optional<std::string> parse_args(int argc, char *argv[]) {
  std::string arg;
  for (int i = 1; i < argc; i++) {
    arg = argv[i];
    if (arg == "-j" || arg == "json") {
      ensure_value(argc, argv, i);
      return std::string(argv[++i]);

    } else if (arg == "-d" || arg == "--debug") {
      spdlog::set_level(spdlog::level::debug);
    } else if (arg == "-q" || arg == "--quiet") {
      spdlog::set_level(spdlog::level::off);
    } else if (arg == "-h" || arg == "--help") {
      print_usage();
    }
  }
  return std::nullopt;
}

void print_usage() { std::cout << "hello"; }

std::string str_to_upper(const std::string &str) {
  std::string str_copy =
      str; // Create a copy to avoid modifying the original string
  for (char &c : str_copy) {
    if (c >= 'a' && c <= 'z') {
      c -= 32; // Convert to uppercase
    }
  }
  return str_copy;
}
std::string str_to_lower(const std::string &str) {
  std::string str_copy =
      str; // Create a copy to avoid modifying the original string
  for (char &c : str_copy) {
    if (c >= 'A' && c <= 'Z') {
      c += 32; // Convert to uppercase
    }
  }
  return str_copy;
}