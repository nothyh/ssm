#ifndef SSM_USER_INPUT_H
#define SSM_USER_INPUT_H
#include <string>
#include <set>
class UserInput
{
public:
    virtual ~UserInput() = default;
    virtual std::string get_mcu_family() = 0;
    virtual std::set<std::string> get_peripherals() = 0;
    virtual std::string get_project_path() = 0;
};

class GuiInput : public UserInput
{
public:
    std::string get_mcu_family() override;
    std::set<std::string> get_peripherals() override;
};

enum class InputMode
{
    CLI,
    GUI
};

#endif