//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/preview_panel.h"

namespace ImGuiEx::M3
{
class DockedToolbarScope;
}

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
    void DrawToolBar(FontBuilder &fontBuilder, Settings &settings);
    void DrawToolBarButtons(const ImGuiEx::M3::DockedToolbarScope &toolBar, FontBuilder &fontBuilder, Settings &settings);

    FontPreviewPanel m_PreviewPanel{};
};
} // namespace Ime::UI
