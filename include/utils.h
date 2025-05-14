#ifndef SSM_UTILS_H
#define SSM_UTILS_H
#include <optional>
#include <string>
std::optional<std::string> parse_args(int argc, char *argv[]);
void print_usage();

std::string str_to_upper(const std::string &str);
std::string str_to_lower(const std::string &str);
#endif