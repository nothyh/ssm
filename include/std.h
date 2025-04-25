#ifndef SSM_STD_H_
#define SSM_STD_H_
#include <string>
class Std
{
private:
public:
    virtual std::string download_std_file(const std::string &url, const std::string &path) = 0;
    virtual void extract_modules(const std::string &pack_path, const std::string &output_path) = 0;
};
#endif