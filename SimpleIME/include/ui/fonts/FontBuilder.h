//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "fonts/FontManager.h"
#include "ui/Settings.h"
#include "ui/fonts/ImFontWrap.h"

struct ImFont;
struct ImGuiTextFilter;

namespace Ime
{
class FontBuilder
{
public:
    [[nodiscard]] constexpr auto IsBuilding() const -> bool;

    auto AddFont(int fontId, ImFontWrap &imFont) -> bool;

    /**
     * Apply current build font to the default @c ImGui font
     * @return is applied?
     */
    auto ApplyFont(Settings &settings) -> bool;

    void Reset()
    {
        m_baseFont.Cleanup();
        m_usedFontIds.clear();
    }

    [[nodiscard]] auto GetBaseFont() const -> const ImFontWrap & { return m_baseFont; }

    [[nodiscard]] constexpr auto GetFontManager() const -> const Fonts::FontManager & { return m_fontManager; }

private:
    Fonts::FontManager m_fontManager;
    ImFontWrap         m_baseFont;
    std::vector<int>   m_usedFontIds; // font index in FontManger#fontInfo list.
};
} // namespace Ime
