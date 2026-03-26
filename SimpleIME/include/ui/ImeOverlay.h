//
// Created by jamie on 2026/3/10.
//

#pragma once

#include "LanguageBar.h"
#include "ToolWindow.h"
#include "i18n/Translator.h"

namespace Ime
{
struct LangProfile;

namespace UI
{
/**
 * @brief Top-level IME overlay. NOT a UI/Menu itself, but a controller for the IME UI layer.
 * Holds the translator lifecycle, handles the toggle shortcut, and renders the always-visible @c LanguageBar.  When the user requests settings, it
 * creates a @c ToolWindow on demand (and destroys it when settings are dismissed).
 *
 *  ImeOverlay
 *    ↓
 *  LanguageBar
 *    ↓
 *  ToolWindow (only when settings are open)
 */
class ImeOverlay
{
    ::i18n::Translator          m_translator{};
    std::unique_ptr<ToolWindow> m_toolWindow{nullptr};
    ImGuiKeyChord               m_shortcut = ImGuiKey_None;

public:
    ImeOverlay(ImGuiKeyChord shortcut, std::string_view language);
    ~ImeOverlay();
    ImeOverlay(const ImeOverlay &other)            = delete;
    ImeOverlay &operator=(const ImeOverlay &other) = delete;

    // Move is deleted: the constructor registers &m_translator with a global pointer;
    // moving the object would leave that pointer dangling.
    ImeOverlay(ImeOverlay &&other) noexcept   = delete;
    ImeOverlay &operator=(ImeOverlay &&other) = delete;

    auto Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, Settings &settings) -> void;
};
} // namespace UI

} // namespace Ime
