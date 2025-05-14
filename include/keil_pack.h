#ifndef SSM_INCLUDE_KEIL_PACK_H
#define SSM_INCLUDE_KEIL_PACK_H
#include "mcu.h"
#include "pack.h"
#include <string>

const std::string PACK_INDEX_FILE_URL = "https://www.keil.com/pack/index.pidx";
class XMLEntry {
public:
  std::string url;
  std::string vendor;
  std::string name;
  std::string version;
};
class KeilPack : public Pack {
private:
  const AllConfig &all_config;
  // https://www.keil.com/pack/Keil.STM32F1xx_DFP.2.4.1.pack
  // STM32F4xx_DFP
  std::string short_name;
  std::string pack_name;
  // https://www.keil.com/pack/Keil.STM32F1xx_DFP.2.4.1.pack
  std::string full_url;
  fs::path pack_path;
  fs::path unzip_path;
  fs::path ssm_path;
  XMLEntry xml_entry;

  bool find_existing_pack();

public:
  KeilPack(const AllConfig &all_config);
  ~KeilPack();
  std::string download_index_file_to_string();
  void set_full_pack_url(const std::string &index_file_in_xml);
  void download_pack(const std::string &dest);
  // void extract_files(const std::string &pack_path,)
  void unzip() override;
  void parse();
};

#endif