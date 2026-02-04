//
// Created by jamie on 2026/1/14.
//

#pragma once

#include "common/toml/toml.hpp"
#include "ui/Settings.h"

#include <filesystem>

namespace Ime::ConfigSerializer
{
void Serialize(const std::filesystem::path &filePath, Settings &settings);

void Deserialize(const std::filesystem::path &filePath, Settings &settings);
} // namespace Ime::ConfigSerializer
