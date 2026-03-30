//
// Created by jamie on 2026/1/28.
//

#pragma once

#include "i18n/Translator.h"

#include <filesystem>
#include <string_view>
#include <vector>

namespace i18n
{
/// Naming pattern for translation files. The `{}` placeholder is replaced by the language name.
static constexpr std::string_view TRANSLATE_FILE_PATTERN = "translate_{}.toml";

/// Regex pattern used to discover translation files. Must match TRANSLATE_FILE_PATTERN.
static constexpr std::string_view TRANSLATE_FILE_REGEX = R"(translate_(\w+)\.toml)";

auto GetTranslator() -> ::i18n::Translator *;
auto SetTranslator(::i18n::Translator *translator) -> void;

inline auto UpdateTranslator(::i18n::Translator &&newTranslator) -> void
{
    auto *current = GetTranslator();
    if (current != nullptr)
    {
        *current = std::move(newTranslator);
    }
}

/**
 * @brief Load a translation for @p language (or @p fallbackLanguage) from @p dir and hot-swap it in.
 *
 * Translation files are expected to be named according to @p filePattern,
 * where the `{}` placeholder is substituted with the language name.
 *
 * @param language         Preferred language name (e.g. "chinese").
 * @param fallbackLanguage Fallback language if the preferred file is missing (e.g. "english").
 * @param dir              Directory that contains the translation files.
 * @param filePattern      File naming pattern. Defaults to @c TRANSLATE_FILE_PATTERN.
 */
auto UpdateTranslator(
    std::string_view language, std::string_view fallbackLanguage, const std::filesystem::path &dir,
    std::string_view filePattern = TRANSLATE_FILE_PATTERN
) -> void;

/**
 * @brief Scan @p dir for available translation files and populate @p languages.
 *
 * @param dir         Directory to scan.
 * @param languages   Output vector; cleared before population.
 * @param filePattern File naming pattern used to identify and extract language names.
 *                    Defaults to @c TRANSLATE_FILE_PATTERN.
 */
void ScanLanguages(const std::filesystem::path &dir, std::vector<std::string> &languages, std::string_view regex = TRANSLATE_FILE_REGEX);

} // namespace i18n

/// Look up @p key in the active translator. Returns @p key as-is if no translator is loaded.
constexpr auto Translate(const std::string_view key) -> std::string_view
{
    auto *translator = ::i18n::GetTranslator();
    if (translator == nullptr)
    {
        return key;
    }
    return translator->Translate(::i18n::HashKey(key), key);
}

/// Look up @p key in the active translator. Returns @p key as-is if no translator is loaded.
constexpr auto Translate1(const std::string_view key) -> const char *
{
    return Translate(key).data();
}
