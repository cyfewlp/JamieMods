//
// Created by jamie on 2026/1/28.
//

#include "i18n/TranslationLoader.h"

#include "common/log.h"
#include "common/toml++/toml.hpp"

#include <iostream>
#include <regex>

void Ime::TranslationLoader::ScanLanguages(const std::filesystem::path &dir, std::vector<std::string> &languages)
{
    namespace fs = std::filesystem;
    const std::regex pattern(R"(translate_(\w+)\.toml)", std::regex_constants::icase);
    std::smatch      themeNameMatch;

    if (!fs::exists(dir) || !fs::is_directory(dir))
    {
        return;
    }

    languages.clear();
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file())
        {
            const std::string filename = entry.path().filename().string();
            if (std::regex_match(filename, themeNameMatch, pattern))
            {
                if (themeNameMatch.size() > 1)
                {
                    languages.push_back(themeNameMatch[1].str());
                }
            }
        }
    }
}

auto Ime::TranslationLoader::LoadFrom(std::string_view language) const -> std::optional<i18n::Translator>
{
    if (!std::filesystem::is_directory(m_dirTranslateFiles))
    {
        return std::nullopt;
    }

    auto translateFile = m_dirTranslateFiles / std::format("translate_{}.toml", language);
    if (!std::filesystem::exists(translateFile))
    {
        return std::nullopt;
    }

    return LoadFromFile(translateFile);
}

void ProcessTable(
    toml::impl::wrap_node<toml::table> *a_table, const std::string &parentKey,
    i18n::Translator::LanguageMap &languageMap
)
{
    if (!a_table) return;
    for (auto it = a_table->begin(); it != a_table->end(); ++it)
    {
        auto        key = it->first.str();
        std::string fullKey;
        if (parentKey.empty())
        {
            fullKey = key;
        }
        else
        {
            fullKey = std::format("{}.{}", parentKey, key);
        }
        if (it->second.is_string())
        {
            languageMap.emplace(i18n::HashKey(fullKey), it->second.value_or(""));
        }
        else if (it->second.is_table())
        {
            ProcessTable(it->second.as_table(), fullKey, languageMap);
        }
    }
}

auto Ime::TranslationLoader::LoadFromFile(const std::filesystem::path &file) const -> std::optional<i18n::Translator>
{
    try
    {
        auto table = toml::parse_file(file.generic_string());
        if (table.empty())
        {
            return std::nullopt;
        }

        i18n::Translator::LanguageMap languageMap;

        ProcessTable(table[m_sectionMain].as_table(), m_sectionMain, languageMap);

        return std::optional(i18n::Translator(std::move(languageMap)));
    }
    catch (std::exception &e)
    {
        logger::warn("Load translate file from {} fail! {}", file.generic_string(), e.what());
    }
    catch (...)
    {
        logger::warn("Load translate file from {} fail!", file.generic_string());
    }
    return std::nullopt;
}
