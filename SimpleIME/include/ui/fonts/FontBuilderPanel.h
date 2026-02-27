//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "imguiex/Material3.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/preview_panel.h"

struct ImGuiTextFilter;

namespace Ime::UI
{
class FontBuilderPanel
{
    static constexpr auto TITLE_HELP    = "Help";
    static constexpr auto TITLE_WARNING = "Warning";

public:
    explicit FontBuilderPanel() = default;

    void Draw(FontBuilder &fontBuilder, Settings &settings);

private:
    // FIXME: refactor
    static void DrawFontInfoTable(const FontBuilder &fontBuilder);
    void        DrawToolBar(FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles);
    void        DrawToolBarButtons(FontBuilder &fontBuilder, Settings &settings);
    static void DrawHelpModal();
    static void DrawWarningsModal();

    FontPreviewPanel m_PreviewPanel{};
};
} // namespace Ime::UI
