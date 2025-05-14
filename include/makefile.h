#ifndef SSM_INCLUDE_MAKEFILE_H
#define SSM_INCLUDE_MAKEFILE_H

#include "mcu.h"
#include <string>
class Makefile {
  // 包括 基本的和外设
  std::string c_sources;
  // 固定
  std::string asm_sources;

  // 来自json
  std::string cpu;
  std::string fpu;
  std::string fpu_float_api;

  // 来自json
  std::string c_defs;
  //
  std::string c_includes;
  // 来自json
  std::string ld_script;
  void render_makefile(const AllConfig &all_config,
                       const std::string &template_path) const;
};

#endif
