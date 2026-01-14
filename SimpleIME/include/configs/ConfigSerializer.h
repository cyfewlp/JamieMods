//
// Created by jamie on 2026/1/14.
//

#pragma once

#include "common/config.h"
#include "common/toml/toml.hpp"
#include "ui/Settings.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ConfigSerializer
{
    static constexpr auto CONFIG_FILE_NAME = "SimpleIME.toml";

public:
    static void Serialize(Settings &settings);
    static void Deserialize(Settings &settings);

private:
    static void DoDeserialize(toml::value &config, Settings &settings);
    static auto DoSerialize(Settings &settings) -> toml::value;
};
}
}