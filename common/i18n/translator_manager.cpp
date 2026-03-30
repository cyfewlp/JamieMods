//
// Created by jamie on 2026/1/28.
//

#include "i18n/translator_manager.h"

#include "log.h"
#include "toml++/toml.hpp"

#include <format>
#include <regex>

namespace i18n
{
namespace
{

::i18n::Translator *g_translator = nullptr;

void ProcessTable(toml::impl::wrap_node<toml::table> *a_table, const std::string &parentKey, ::i18n::Translator::LanguageMap &languageMap)
{
    if (a_table == nullptr) return;
    for (auto it = a_table->begin(); it != a_table->end(); ++it)
    {
        const auto key     = it->first.str();
        const auto fullKey = parentKey.empty() ? std::string(key) : std::format("{}.{}", parentKey, key);

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
            const auto key = pair.first.str();
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
                logger::warn("i18n: Unknown value type '{}' for key '{}'. Check your translation file.", toml::impl::to_sv(pair.second.type()), key);
            }
        }

        return std::make_optional(::i18n::Translator(std::move(languageMap)));
    }
    catch (const std::exception &e)
    {
        logger::warn("i18n: Failed to load '{}': {}", file.generic_string(), e.what());
    }
    catch (...)
    {
        logger::warn("i18n: Failed to load '{}'.", file.generic_string());
    }
    return std::nullopt;
}

auto LoadTranslation(std::string_view language, const std::filesystem::path &dir, std::string_view filePattern) -> std::optional<::i18n::Translator>
{
    if (!std::filesystem::is_directory(dir))
    {
        return std::nullopt;
    }

    const auto translateFile = dir / std::vformat(filePattern, std::make_format_args(language));
    if (!std::filesystem::exists(translateFile))
    {
        return std::nullopt;
    }
    return LoadFromFile(translateFile);
}

} // namespace

auto GetTranslator() -> ::i18n::Translator *
{
    return g_translator;
}

auto SetTranslator(::i18n::Translator *translator) -> void
{
    g_translator = translator;
}

auto UpdateTranslator(std::string_view language, std::string_view fallbackLanguage, const std::filesystem::path &dir, std::string_view filePattern)
    -> void
{
    if (auto translator = LoadTranslation(language, dir, filePattern); translator.has_value())
    {
        UpdateTranslator(std::move(translator.value()));
    }
    else if (auto fallback = LoadTranslation(fallbackLanguage, dir, filePattern); fallback.has_value())
    {
        UpdateTranslator(std::move(fallback.value()));
    }
    else
    {
        logger::warn("i18n: Failed to load translation for '{}' (fallback '{}') in '{}'.", language, fallbackLanguage, dir.generic_string());
    }
}

void ScanLanguages(const std::filesystem::path &dir, std::vector<std::string> &languages, std::string_view regex)
{
    namespace fs = std::filesystem;

    if (!fs::exists(dir) || !fs::is_directory(dir))
    {
        return;
    }

    const std::regex pattern(regex.data(), std::regex_constants::icase);
    std::smatch      match;

    languages.clear();
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file()) continue;

        const std::string filename = entry.path().filename().string();
        if (std::regex_match(filename, match, pattern) && match.size() > 1)
        {
            languages.push_back(match[1].str());
        }
    }
}

} // namespace i18n
