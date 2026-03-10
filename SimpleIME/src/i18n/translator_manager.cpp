//
// Created by jamie on 2026/1/28.
//

#include "i18n/translator_manager.h"

#include "log.h"
#include "toml++/toml.hpp"

#include <iostream>
#include <regex>

namespace Ime::i18n
{
namespace
{
auto LoadFromFile(const std::filesystem::path &file) -> std::optional<::i18n::Translator>;
void ProcessTable(toml::impl::wrap_node<toml::table> *a_table, const std::string &parentKey, ::i18n::Translator::LanguageMap &languageMap);

auto LoadTranslation(Language language, const std::filesystem::path &dir) -> std::optional<::i18n::Translator>
{
    if (!std::filesystem::is_directory(dir))
    {
        return std::nullopt;
    }

    auto translateFile = dir / std::format("translate_{}.toml", language);
    if (!std::filesystem::exists(translateFile))
    {
        return std::nullopt;
    }
    return LoadFromFile(translateFile);
}

} // namespace

void ScanLanguages(const std::filesystem::path &dir, std::vector<std::string> &languages)
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

auto GetTranslator() -> std::unique_ptr<::i18n::Translator> &
{
    static std::unique_ptr<::i18n::Translator> g_translator{nullptr};
    return g_translator;
}

auto UpdateTranslator(std::string_view language, std::string_view fallbackLanguage, const std::filesystem::path &dir) -> void
{
    if (auto translator = LoadTranslation(language, dir); translator.has_value())
    {
        UpdateTranslator(std::move(translator.value()));
    }
    else if (auto fallbackTranslator = LoadTranslation(fallbackLanguage, dir); fallbackTranslator.has_value())
    {
        UpdateTranslator(std::move(fallbackTranslator.value()));
    }
    else
    {
        logger::warn("Failed to load translator for '{}' (and fallback '{}'). Translation will be unavailable.", language, fallbackLanguage);
    }
}

namespace
{
void ProcessTable(toml::impl::wrap_node<toml::table> *a_table, const std::string &parentKey, ::i18n::Translator::LanguageMap &languageMap)
{
    if (a_table == nullptr) return;
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
            languageMap.emplace(::i18n::HashKey(fullKey), it->second.value_or(""));
        }
        else if (it->second.is_table())
        {
            ProcessTable(it->second.as_table(), fullKey, languageMap);
        }
    }
}

auto LoadFromFile(const std::filesystem::path &file) -> std::optional<::i18n::Translator>
{
    try
    {
        auto table = toml::parse_file(file.generic_string());
        if (table.empty())
        {
            return std::nullopt;
        }

        ::i18n::Translator::LanguageMap languageMap;

        for (const auto &pair : table)
        {
            auto key = pair.first.str();
            if (pair.second.is_table())
            {
                ProcessTable(pair.second.as_table(), std::string(key), languageMap);
            }
            else if (pair.second.is_string())
            {
                languageMap.emplace(::i18n::HashKey(key), pair.second.value_or(""));
            }
            else
            {
                logger::warn(
                    "i18n: Unknown value type '{}' encountered for key '{}'. Please check your translation file or "
                    "update the mod.",
                    toml::impl::to_sv(pair.second.type()),
                    key
                );
            }
        }

        return std::optional(::i18n::Translator(std::move(languageMap)));
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

} // namespace
} // namespace Ime::i18n
