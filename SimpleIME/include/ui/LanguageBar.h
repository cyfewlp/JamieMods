//
// Created by jamie on 2026/2/6.
//

#pragma once

#include "imgui.h"
#include "tsf/LangProfile.h"

#include <vector>

namespace Ime
{

class LanguageBar
{
    bool          m_pinned   = false;
    bool          m_showing  = false;
    ImGuiKeyChord m_shortCut = ImGuiKey_None;

public:
    auto Draw(const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> bool;

    [[nodiscard]] auto IsPinned() const -> bool { return m_pinned; }

    [[nodiscard]] auto IsShowing() const -> bool { return m_showing; }

    void SetShortCut(ImGuiKeyChord key) { m_shortCut = key; }

private:
    auto DoDraw(bool &openSettings, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> void;
};

} // namespace Ime
