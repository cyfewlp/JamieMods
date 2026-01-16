//
// Created by jamie on 2026/1/16.
//

#pragma once

#include "common/config.h"
#include "ui/fonts/ImFontWrap.h"

struct ImGuiTextFilter;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class FontPreviewPanel
{
    ImFontWrap      m_imFont{nullptr, "", ""};
    ImGuiTextFilter m_filter = {};

public:
    bool Draw(FontBuilder &fontBuilder, const Translation &translation, const Settings &settings);

    void PreviewFont(const std::string &fontName, const std::string &fontPath);

    void PreviewFont(const ImFontWrap &imFont)
    {
        m_imFont = imFont;
    }

    void Cleanup();

    [[nodiscard]] auto GetImFont() const -> const ImFontWrap &
    {
        return m_imFont;
    }

    [[nodiscard]] auto GetImFont() -> ImFontWrap &
    {
        return m_imFont;
    }
};
}
}