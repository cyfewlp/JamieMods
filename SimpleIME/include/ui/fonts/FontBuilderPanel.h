//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "common/imgui/Material3.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontPreviewPanel.h"

struct ImGuiTextFilter;

namespace Ime
{
class FontBuilderPanel
{
    static constexpr auto TITLE_HELP    = "Help";
    static constexpr auto TITLE_WARNING = "Warning";

public:
    explicit FontBuilderPanel() = default;

    void Draw(FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles);

private:
    void        DrawAddFontButton(FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles);
    void        DrawFontInfoTable(const FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles) const;
    void        DrawToolBar(FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles);
    void        DrawToolBarButtons(FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles);
    static void DrawHelpModal();
    static void DrawWarningsModal();

    FontPreviewPanel m_PreviewPanel{};
};
} // namespace Ime
