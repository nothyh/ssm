#include "keil_pack.h"
#include <curl/curl.h>
#include "pugiconfig.hpp"
#include "pugixml.hpp"
#include <iostream>
#include <cstdio>
KeilPack::KeilPack(const std::string &index_url, const std::string &pack_name)
    : pack_index_url(index_url), pack_name(pack_name)
{
    // Constructor implementation
}
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    std::string *str = (std::string *)userp;
    str->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string KeilPack::download_index_to_string()
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }
    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, pack_index_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
    return readBuffer;
}

std::string KeilPack::get_full_pack_url(const std::string &xml)
{
    pugi::xml_document doc;
    pugi::xml_parse_result parse_result = doc.load_string(xml.c_str());
    if (!parse_result)
    {
        std::cerr << "Failed to parse XML: " << parse_result.description() << std::endl;
        return "";
    }
    std::string xPath = "//pindex/pdsc[@name='" + this->pack_name + "']";
    pugi::xpath_node_set nodes = doc.select_nodes(xPath.c_str());
    if (nodes.empty())
    {
        std::cerr << "No nodes found for XPath: " << xPath << std::endl;
        return "";
    }
    if (nodes.size() > 1)
    {
        std::cerr << "Multiple nodes found for XPath: " << xPath << std::endl;
        return "";
    }
    pugi::xml_node node = nodes.first().node();
    std::string url = node.attribute("url").as_string();
    std::string version = node.attribute("version").as_string();
    std::string vendor = node.attribute("vendor").as_string();
    this->pack_full_url = url + vendor + "." + this->pack_name + "." + version + ".pack";
    return this->pack_full_url;
}
std::string KeilPack::download_pack(const std::string &url, const std::string &path)
{
    CURL *curl = curl_easy_init();
    FILE *fp = fopen(path.c_str(), "wb+");
    if (!fp)
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }

    if (!curl)
    {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        fclose(fp);
        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
    fclose(fp);
    return path;
}