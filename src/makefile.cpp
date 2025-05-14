
#include "makefile.h"
#include "mcu.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

void Makefile::render_makefile(const AllConfig &all_config,
                               const std::string &template_path) const {
  std::ifstream fin(template_path);
  if (!fin.is_open()) {
    throw std::runtime_error("can not open template file");
  }
  std::stringstream buffer;
  buffer << fin.rdbuf();
  std::string content = buffer.str();
  std::unordered_map<std::string, std::string> replacements = {
      {"{{C_SOURCES}}", c_sources},
      {"{{ASM_SOURCES}}", asm_sources},
      {"{{CPU}}", cpu},
      {"{{FPU}}", fpu},
      {"{{FLOAT-API}}", fpu_float_api},
      {"{{C_DEFS}}", c_defs},
      {"{{C_INCLUDES}}", c_includes},
      {"{{LD_SCRIPT}}", ld_script}};

  for (const auto &[key, value] : replacements) {
    size_t pos = 0;
    while ((pos = content.find(key, pos) != std::string::npos)) {
      content.replace(pos, key.length(), value);
      pos += value.length();
    }
  }
  fs::path make_path = all_config.project_path / "Makefile";
  std::ofstream make_out(make_path);
  make_out << content;
  make_out.close();
}