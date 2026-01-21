//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "common/config.h"
#include "ui/Settings.h"
#include "ui/fonts/FontManager.h"
#include "ui/fonts/ImFontWrap.h"

struct ImFont;
struct ImGuiTextFilter;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class FontBuilder
{
public:
    void Initialize()
    {
        m_fontManager.FindInstalledFonts();
    }

    constexpr bool IsBuilding() const;

    bool AddFont(int fontId, ImFontWrap &imFont);

private:
    bool MergeFont(ImFontWrap &imFontWrap);

public:
    /**
     * Apply current build font to the default @c ImGui font
     * @return is applied?
     */
    bool ApplyFont(Settings &settings);

    void Reset()
    {
        m_baseFont.Cleanup();
        m_usedFontIds.clear();
    }

    [[nodiscard]] auto GetBaseFont() const -> const ImFontWrap &
    {
        return m_baseFont;
    }

    constexpr auto GetFontManager() const -> const FontManager &
    {
        return m_fontManager;
    }

private:
    FontManager      m_fontManager = {};
    ImFontWrap       m_baseFont{};
    std::vector<int> m_usedFontIds; // font index in FontManger#fontInfo list.
};
}
}
