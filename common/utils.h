//
// Created by jamie on 2025/5/26.
//

#ifndef UTILS_H
#define UTILS_H

#include <filesystem>
#include <format>
#include <optional>
#include <string>

namespace utils
{
#if defined(ENABLE_SKYRIM_VR) || defined(ENABLE_SKYRIM_SE) || defined(ENABLE_SKYRIM_AE)
    #include "SKSE/Interfaces.h"

constexpr auto GetInterfaceFile(const std::string_view &fileName) -> std::string
{
    const auto &pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    return std::format("Data\\interface\\{}\\{}", pluginName, fileName);
}
#endif

constexpr auto GetInterfacePath() -> std::filesystem::path
{
    return "Data/interface";
}

constexpr uint8_t HexCharToInt(char c)
{
    if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
    if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
    return UINT8_MAX;
}
} // namespace utils

#endif // UTILS_H
