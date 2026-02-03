//
// Created by jamie on 2026/1/28.
//

#pragma once

#include "common/i18n/Translator.h"

#include <filesystem>

namespace Ime
{
class TranslationLoader
{
    std::filesystem::path m_dirTranslateFiles;
    std::string           m_sectionMain;

public:
    explicit TranslationLoader(const std::filesystem::path &dirTranslateFiles, std::string_view sectionMain)
        : m_dirTranslateFiles(dirTranslateFiles), m_sectionMain(sectionMain)
    {
    }

    static void ScanLanguages(const std::filesystem::path &dir, std::vector<std::string> &languages);

    auto LoadFrom(std::string_view language) const -> std::optional<i18n::Translator>;

private:
    auto LoadFromFile(const std::filesystem::path &file) const -> std::optional<i18n::Translator>;
};
} // namespace Ime
