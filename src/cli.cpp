#include "cli.h"
#include <iostream>
#include <string>
#include <sstream>
std::string CliInput::get_mcu_family()
{
    std::string mcu_family;
    std::cout << "Enter the MCU family: ";
    std::getline(std::cin, mcu_family);

    return mcu_family;
}
std::set<std::string> CliInput::get_peripherals()
{
    std::set<std::string> peripherals;
    std::string input;
    std::cout << "Enter the peripherals (space-separated): ";
    std::getline(std::cin, input);
    std::istringstream iss(input);
    std::string peripheral;
    while (iss >> peripheral)
    {
        peripherals.insert(peripheral);
    }
    return peripherals;
}

std::string CliInput::get_project_path()
{
    std::string project_path;
    std::cout << "Enter the project path: ";
    std::getline(std::cin, project_path);
    return project_path;
}