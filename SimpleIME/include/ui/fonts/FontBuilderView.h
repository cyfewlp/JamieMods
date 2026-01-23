//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "common/config.h"
#include "common/imgui/Material3.h"
#include "core/Translation.h"
#include "ui/Settings.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontPreviewPanel.h"

struct ImGuiTextFilter;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class FontBuilderView
{
    static constexpr auto TITLE_HELP     = "Help";
    static constexpr auto TITLE_WARNINGS = "Warnings";

public:
    explicit FontBuilderView(ImGuiEx::M3::M3Styles &styles) : m_styles(styles), m_PreviewPanel(styles) {}

    void Draw(FontBuilder &fontBuilder, const Translation &translation, Settings &settings);

private:
    void        DrawAddFontButton(FontBuilder &fontBuilder, const Translation &translation);
    void        DrawFontInfoTable(const FontBuilder &fontBuilder) const;
    void        DrawToolBar(FontBuilder &fontBuilder, const Translation &translation, Settings &settings);
    void        DrawToolBarButtons(FontBuilder &fontBuilder, const Translation &translation, Settings &settings);
    static void DrawHelpModal(const Translation &translation);
    static void DrawWarningsModal(const Translation &translation);

    ImGuiEx::M3::M3Styles &m_styles;
    FontPreviewPanel       m_PreviewPanel;
};
}
}