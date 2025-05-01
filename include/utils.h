#ifndef SSM_UTILS_H
#define SSM_UTILS_H
#include <string>
#include <vector>
void create_directory(const std::string &path);
void validate_file(const std::string &file_name);
std::vector<std::string> str_split(std::string_view str, std::string_view delim);
#endif