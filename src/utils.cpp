#include "utils.h"
#include <filesystem>
namespace fs = std::filesystem;
#include <iostream>

void create_directory(const std::string &path)
{
    const fs::path dir_path{path};
    if (!fs::exists(dir_path))
    {
        try
        {
            fs::create_directories(dir_path);
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "Error creating directory: " << e.what() << '\n';
        }
    }
    else
    {
        std::cout << "Directory already exists: " << path << '\n';
    }
}

void validate_file(const std::string &file_name)
{
    const fs::path file_path{file_name};
    if (!fs::exists(file_path))
    {
        std::cerr << "File does not exist: " << file_name << '\n';
        throw std::runtime_error("File does not exist");
    }
    else
    {
        std::cout << "File exists: " << file_name << '\n';
    }
}