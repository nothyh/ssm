#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <set>
#include "main.h"
#include "keil_pack.h"
#include "arm_std.h"
#include "arg_parse.h"
#include "cli.h"
#include "logger.h"
std::string user_input()
{
    std::string input = "gpio timer";
    return input;
}

std::string read_file_to_string(const std::string &filename)
{
    std::ifstream makefile_template(filename, std::ios::in | std::ios::binary);
    if (!makefile_template.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }
    std::stringstream content;
    content << makefile_template.rdbuf();
    if (makefile_template.bad())
    {
        makefile_template.close();
        std::cerr << "Error reading file: " << filename << std::endl;
        return "";
    }
    return content.str();
}

void string_replace(std::string &content, const std::string &from, const std::string &to)
{
    size_t pos = 0;
    while ((pos = content.find(from, pos)) != std::string::npos)
    {
        content.replace(pos, from.length(), to);
        pos += to.length(); // Move past the replaced part to avoid infinite loop
    }
}

bool generate_makefile()
{
    std::string makefile_template;
    return false;
}

class MCU
{
public:
    std::string name;
    std::string family;
    std::string architecture;
    std::set<std::string> peripherals;
};

std::set<std::string>
get_userinput()
{
    return std::set<std::string>{"gpio", "timer"};
}

int main(int argc, char *argv[])
{
    Logger::instance().enable_debug();

    ArgParser arg_parser(argc, argv);
    std::unique_ptr<UserInput> input;
    if (arg_parser.input_mode == InputMode::CLI)
    {
        input = std::make_unique<CliInput>();
    }

    ArmStd arm_std(std::move(input));
    const std::string zip_file_path{"/home/hyh/workspace/proj/ssm/ref/stdlib/en.stsw-stm32054_v3-6-0.zip"};
    arm_std.extrect_lib_to_tmp(zip_file_path);
    arm_std.get_from_lib_to_proj(R"(.*/STM32.*/Lib.*)");
    arm_std.construct_core_dir();

    return 0;
}