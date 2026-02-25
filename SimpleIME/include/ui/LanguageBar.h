//
// Created by jamie on 2026/2/6.
//

#pragma once

#include "imguiex/Material3.h"
#include "tsf/LangProfile.h"

#include <vector>

namespace Ime
{

class LanguageBar
{
    bool m_pinned  = false;
    bool m_showing = false;

public:
    auto Draw(bool wantToggle, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, ImGuiEx::M3::M3Styles &m3Styles)
        -> bool;

    [[nodiscard]] auto IsPinned() const -> bool { return m_pinned; }

    [[nodiscard]] auto IsShowing() const -> bool { return m_showing; }

private:
    auto DoDraw(
        bool &openSettings, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles, const ImGuiEx::M3::M3Styles &m3Styles
    ) -> void;
};

} // namespace Ime
