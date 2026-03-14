//
// Created by jamie on 2026/3/8.
//

#pragma once

namespace Ime
{
struct Settings;
struct Configuration;

auto ConvertConfigurationToSettings(const Configuration &config) -> Settings;
auto ConvertSettingsToConfiguration(const Settings &settings) -> Configuration;

} // namespace Ime
