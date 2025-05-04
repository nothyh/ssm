#ifndef SSM_ARM_STD_H
#define SSM_ARM_STD_H
#include "arm_std.h"
#include <string>
#include "user_input.h"
#include <set>
#include <memory>
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;
class ArmStd
{
private:
    std::string mcu_family;
    std::set<std::string> user_peripherals;
    fs::path project_path;
    std::unique_ptr<UserInput> user_input;
    fs::path unzip_tmp_dir;
    fs::path core_dir;
    fs::path makefile_path;
    bool extract_lib(const fs::path &zip_file_path, const std::string &to_extract, const fs::path &dest_path);
    bool get_from_lib_to(const std::string &prefix_pattern, const std::string &dest_path);
    void create_makefile();
    void modify_makefile();
    std::string peripherals_to_file();
    std::string get_c_sources();
    std::string get_asm_sources();
    std::string get_c_defs();

    std::string get_c_includes();
    bool copy_from_to(const fs::path &src, const fs::path &dest);
    std::string get_float_abi();
    std::string get_fpu();
    std::string get_cpu();
    std::string get_ld_script();

public:
    ArmStd(std::unique_ptr<UserInput> input);
    ~ArmStd();

    void set_mcu_family();
    void set_peripherals();
    void set_project_path();
    std::string get_project_path() const { return project_path; }
    std::string download_std_file(const std::string &file_name);
    bool extrect_lib_to_tmp(const fs::path &zip_file_path);

    bool get_from_lib_to_proj(const std::string &prefix_pattern);
    bool construct_core_dir();
    fs::path get_unzip_gmp_dir();
    void construct_makefile();
};
#endif