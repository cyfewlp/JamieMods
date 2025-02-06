#include <cstdint>
#include <iostream>
#include <string>

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
    std::string hexStr = "0xFFFFFFFF"; // 16 进制字符串
    uint32_t    value  = HexStringToUInt32(hexStr);
    std::cout << "Hex: " << hexStr << " -> UInt32: " << value << std::endl;
    return 0;
}