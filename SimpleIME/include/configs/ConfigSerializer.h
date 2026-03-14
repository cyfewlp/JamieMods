//
// Created by jamie on 2026/1/14.
//

#pragma once

#include "ui/Settings.h"

#include <filesystem>

namespace Ime
{

struct Configuration;

namespace ConfigSerializer
{
//! @brief Save the configuration to a file. Will overwrite the file if it already exists.
//! All exceptions will be caught and logged, and the function will not throw.
//! The file shall not be written if any error occurs during serialization, and the original file will be kept intact.
void SaveConfiguration(const std::filesystem::path &filePath, const Configuration &configuration);

auto LoadConfiguration(const std::filesystem::path &filePath) -> Configuration;
} // namespace ConfigSerializer

} // namespace Ime
