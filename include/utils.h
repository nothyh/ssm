#ifndef SSM_UTILS_H
#define SSM_UTILS_H
#include <string>
#include <vector>
void create_directory(const std::string &path);
void validate_file(const std::string &file_name);
std::vector<std::string> str_split(std::string_view str, std::string_view delim);

void string_replace(std::string &content, const std::string &from, const std::string &to);
std::string str_to_upper(const std::string &str);
std::string str_to_lower(const std::string &str);
#endif