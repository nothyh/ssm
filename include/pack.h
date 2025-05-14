#ifndef SSM_INCLUDE_PACK_H
#define SSM_INCLUDE_PACK_H
#include <filesystem>
namespace fs = std::filesystem;

class Pack {
protected:
  bool default_unzip(const fs::path &zip_file, const std::string &prefix,
                     const fs::path &dest);

public:
  virtual void unzip() = 0;
};

#endif
