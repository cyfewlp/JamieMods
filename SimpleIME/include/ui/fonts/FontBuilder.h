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

    void SetBaseFont(ImFontWrap &imFont);
    bool MergeFont(ImFontWrap &imFontWrap);
    /**
     * Apply current build font to the default @c ImGui font
     * @return is applied?
     */
    bool ApplyFont(Settings &settings);

    void Reset()
    {
        m_baseFont.Cleanup();
    }

    [[nodiscard]] auto GetBaseFont() const -> const ImFontWrap &
    {
        return m_baseFont;
    }

    constexpr auto GetFontManager() -> const FontManager &
    {
        return m_fontManager;
    }

private:
    FontManager m_fontManager = {};
    ImFontWrap  m_baseFont{nullptr, "", ""};
};
}
}
