//
// Created by jamie on 2026/1/14.
//

#pragma once

#include "common/config.h"
#include "common/toml/toml.hpp"
#include "ui/Settings.h"

#include <filesystem>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ConfigSerializer
{
public:
    static void Serialize(const std::filesystem::path &filePath, Settings &settings);
    static void Deserialize(const std::filesystem::path &filePath, Settings &settings);

private:
    static void DoDeserialize(toml::value &config, Settings &settings);
    static auto DoSerialize(Settings &settings) -> toml::value;
};
}
}