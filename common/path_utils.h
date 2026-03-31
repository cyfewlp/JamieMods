//
// Created by jamie on 2025/5/26.
//

#pragma once

#include <filesystem>
#include <format>
#include <string>
#include <string_view>

#if defined(ENABLE_SKYRIM_VR) || defined(ENABLE_SKYRIM_SE) || defined(ENABLE_SKYRIM_AE)
    #include "SKSE/Interfaces.h"
#endif

namespace utils
{
#if defined(ENABLE_SKYRIM_VR) || defined(ENABLE_SKYRIM_SE) || defined(ENABLE_SKYRIM_AE)

constexpr auto GetInterfaceFile(const std::string_view &fileName) -> std::string
{
    const auto &pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    return std::format(R"(Data\interface\{}\{})", pluginName, fileName);
}

constexpr auto GetPluginInterfaceDir() -> std::filesystem::path
{
    const auto &pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    return std::format(R"(Data\interface\{})", pluginName);
}
#endif

constexpr auto GetInterfacePath() -> std::filesystem::path
{
    return "Data/interface";
}

} // namespace utils
