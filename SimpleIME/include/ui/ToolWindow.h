//
// Created by jamie on 2026/3/10.
//

#pragma once

#include "LanguageBar.h"
#include "SettingsWindow.h"

namespace Ime
{
struct LangProfile;

namespace UI
{
/**
 * @note Initialize FontManager will be time-consuming operation, it will load all installed system font by DWrite API during constructing.
 */
class ToolWindow
{
    std::unique_ptr<SettingsWindow> m_settingsWindow{nullptr};
    ImGuiKeyChord                   m_shortcut = ImGuiKey_None;
    bool                            m_pinned   = false;
    bool                            m_showing  = false;

public:
    ToolWindow(ImGuiKeyChord shortcut, std::string_view language);
    ~ToolWindow();
    ToolWindow(const ToolWindow &other)            = delete;
    ToolWindow &operator=(const ToolWindow &other) = delete;

    ToolWindow(ToolWindow &&other) noexcept : m_settingsWindow(std::move(other.m_settingsWindow)) {}

    ToolWindow &operator=(ToolWindow &&other) noexcept
    {
        if (this == &other) return *this;
        m_settingsWindow = std::move(other.m_settingsWindow);
        return *this;
    }

    [[nodiscard]] auto IsPinned() const -> bool { return m_pinned; }

    [[nodiscard]] auto IsShowing() const -> bool { return m_showing; }

    auto Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, Settings &settings) -> bool;
};
} // namespace UI

} // namespace Ime
