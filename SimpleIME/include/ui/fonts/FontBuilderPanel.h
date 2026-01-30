//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "common/config.h"
#include "common/imgui/Material3.h"
#include "ui/Settings.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontPreviewPanel.h"

struct ImGuiTextFilter;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class FontBuilderPanel
{
    static constexpr auto TITLE_HELP    = "Help";
    static constexpr auto TITLE_WARNING = "Warning";

public:
    explicit FontBuilderPanel(ImGuiEx::M3::M3Styles &styles) : m_styles(styles), m_PreviewPanel(styles) {}

    void Draw(FontBuilder &fontBuilder, Settings &settings);

private:
    void        DrawAddFontButton(FontBuilder &fontBuilder);
    void        DrawFontInfoTable(const FontBuilder &fontBuilder) const;
    void        DrawToolBar(FontBuilder &fontBuilder, Settings &settings);
    void        DrawToolBarButtons(FontBuilder &fontBuilder, Settings &settings);
    static void DrawHelpModal();
    static void DrawWarningsModal();

    ImGuiEx::M3::M3Styles &m_styles;
    FontPreviewPanel       m_PreviewPanel;
};
}
}