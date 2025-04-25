#ifndef SSM_CLI_H_
#define SSM_CLI_H_
#include <string>
#include "user_input.h"

class CliInput : public UserInput
{
public:
    std::string get_mcu_family() override;
    std::set<std::string> get_peripherals() override;
    std::string get_project_path() override;
};
#endif