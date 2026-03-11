//
// Created by jamie on 2026/1/29.
//

#pragma once

#include "i18n/Translator.h"
#include "path_utils.h"

#include <atomic>
#include <memory>

namespace Ime
{

/**
 * @brief Manages the lifecycle and runtime updates of the translation engine.
 * @note **Intentional Design Trade-offs:**
 * The "unsafe" aspects of this manager are deliberate to satisfy SimpleIME's
 * specific requirements for performance and ease of use:
 * - **Ease of Access:** Call @c Translate() anywhere without managing
 * translator initialization or lifetime.
 * - **Runtime Hot-swapping:** Update languages via @c UpdateTranslator()
 * without complex locking overhead.
 *
 * **Target Scope:**
 * This is not a general-purpose i18n framework; it is a high-efficiency solution
 * tailored specifically for SimpleIME-related string lookups.
 */
namespace i18n
{
using Language = std::string_view;

auto GetTranslator() -> std::unique_ptr<::i18n::Translator> &;

inline auto UpdateTranslator(::i18n::Translator &&newTranslator) -> void
{
    GetTranslator() = std::make_unique<::i18n::Translator>(std::move(newTranslator));
}

auto UpdateTranslator(std::string_view language, std::string_view fallbackLanguage, const std::filesystem::path &dir) -> void;

inline auto UpdateTranslator(std::string_view language, std::string_view fallbackLanguage) -> void
{
    UpdateTranslator(language, fallbackLanguage, utils::GetInterfacePath() / SIMPLE_IME);
}

inline constexpr auto ReleaseTranslator() -> void // FIXME: may access released translator! e.g. dll unload.
{
    GetTranslator().reset();
}

void ScanLanguages(const std::filesystem::path &dir, std::vector<std::string> &languages);
} // namespace i18n

constexpr auto Translate(const std::string_view key) -> std::string_view
{
    auto *translator = i18n::GetTranslator().get();
    if (translator == nullptr)
    {
        return key;
    }
    return translator->Translate(::i18n::HashKey(key), key);
}
} // namespace Ime
