#ifndef PACK_H
#define PACK_H
#include <string>
const std::string PACK_INDEX_URL = "https://www.keil.com/pack/index.pidx";
class KeilPack
{
private:
    // https://www.keil.com/pack/Keil.STM32F1xx_DFP.2.4.1.pack
    std::string pack_index_url;
    std::string pack_base_url;
    std::string pack_name;
    std::string pack_version;
    std::string pack_full_url;

public:
    KeilPack(const std::string &index_url, const std::string &pack_name);
    std::string download_index_to_string();
    std::string get_full_pack_url(const std::string &xml);
    std::string download_pack(const std::string &url, const std::string &path);
    // void extract_files(const std::string &pack_path,)
};
#endif