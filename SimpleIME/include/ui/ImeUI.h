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
 * Top-level IME overlay.  Holds the translator lifecycle, handles the toggle shortcut,
 * and renders the always-visible @c LanguageBar.  When the user requests settings, it
 * creates a @c ToolWindow on demand (and destroys it when settings are dismissed).
 *
 *  ImeUI
 *    ↓
 *  LanguageBar
 *    ↓
 *  ToolWindow (only when settings are open)
 */
class ImeUI
{
    ::i18n::Translator          m_translator{};
    std::unique_ptr<ToolWindow> m_toolWindow{nullptr};
    ImGuiKeyChord               m_shortcut           = ImGuiKey_None;
    bool                        m_pinnedLanguageBar  = false;
    bool                        m_showingLanguageBar = false;

public:
    ImeUI(ImGuiKeyChord shortcut, std::string_view language);
    ~ImeUI();
    ImeUI(const ImeUI &other)            = delete;
    ImeUI &operator=(const ImeUI &other) = delete;

    ImeUI(ImeUI &&other) noexcept : m_toolWindow(std::move(other.m_toolWindow)) {}

    ImeUI &operator=(ImeUI &&other) noexcept
    {
        if (this == &other) return *this;
        m_toolWindow = std::move(other.m_toolWindow);
        return *this;
    }

    [[nodiscard]] auto IsPinned() const -> bool { return m_pinnedLanguageBar; }

    [[nodiscard]] auto IsShowing() const -> bool { return m_showingLanguageBar; }

    auto Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, Settings &settings) -> bool;
};
} // namespace UI

} // namespace Ime
