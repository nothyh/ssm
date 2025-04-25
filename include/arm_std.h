#ifndef SSM_ARM_STD_H
#define SSM_ARM_STD_H
#include "arm_std.h"
#include <string>
#include "user_input.h"
#include <set>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;
class ArmStd
{
private:
    std::string mcu_family;
    std::set<std::string> user_peripherals;
    std::string project_path;
    std::unique_ptr<UserInput> user_input;

public:
    ArmStd(std::unique_ptr<UserInput> input);
    ~ArmStd();

    void set_mcu_family();
    void set_peripherals();
    void set_project_path();
    std::string get_project_path() const { return project_path; }
    std::string download_std_file(const std::string &file_name);
    bool extract_libraries(const fs::path &zipFilePath, const std::string &folderNameToExtract, const fs::path &destinationPath);
};
#endif