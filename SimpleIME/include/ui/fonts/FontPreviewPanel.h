//
// Created by jamie on 2026/1/16.
//

#pragma once

#include "common/config.h"
#include "ui/DebounceTimer.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/ImFontWrap.h"

#include <chrono>

struct ImGuiTextFilter;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class FontPreviewPanel
{
    ImFontWrap      m_imFont{nullptr, "", ""};
    DebounceTimer   m_debounceTimer{300ms};
    ImGuiTextFilter m_filter = {};

public:
    struct InteractState
    {
        bool interact      = false;
        int  selectedIndex = -1;
    };

    auto Draw(FontBuilder &fontBuilder, const Translation &translation, const Settings &settings) -> InteractState;

private:
    void DrawSearchBox();
    void DrawFontsTable(FontBuilder &fontBuilder);

public:
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

private:
    InteractState m_interactState;
};
}
}