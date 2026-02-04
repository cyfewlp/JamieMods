//
// Created by jamie on 2026/1/28.
//

#pragma once

#include "common/i18n/Translator.h"

#include <filesystem>

namespace i18n
{
using Language = std::string_view;

auto LoadTranslation(Language language, const std::filesystem::path &dir) -> std::optional<Translator>;

void ScanLanguages(const std::filesystem::path &dir, std::vector<std::string> &languages);

} // namespace i18n
