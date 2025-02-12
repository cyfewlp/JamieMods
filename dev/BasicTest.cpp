#include <cstdint>
#include <iostream>
#include <string>
#include "yaml-cpp/yaml.h"

uint32_t HexStringToUInt32(const std::string &hexStr)
{
    try
    {
        size_t start = (hexStr.find("0x") == 0 || hexStr.find("0X") == 0) ? 2 : 0;
        return static_cast<uint32_t>(std::stoul(hexStr.substr(start), nullptr, 16));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << " for input: " << hexStr << std::endl;
    }
    return 0;
}

int main()
{
    YAML::Node config = YAML::LoadFile("yaml-aoo.yaml");

    if (config["first"])
    {
        std::cout << "First key: " << config["first"].as<std::string>() << "\n";
    }
    if (config["unsignedTest"])
    {
        std::cout << "unsignedTest: " << config["unsignedTest"].as<std::uint32_t>() << "\n";
    }
    if (config["UI"])
    {
        std::cout << "unsignedTest: " << config["UI"]["k1"].as<std::string>() << "\n";
    }
}