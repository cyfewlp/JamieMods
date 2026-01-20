//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "common/config.h"
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
    void Draw(FontBuilder &fontBuilder, const Translation &translation, Settings &settings);

private:
    static void DrawFontInfoTable(const FontBuilder &fontBuilder);
    void        DrawToolBar(FontBuilder &fontBuilder, const Translation &translation, Settings &settings);
    void        DrawToolBarButtons(FontBuilder &fontBuilder, const Translation &translation, Settings &settings);
    static void DrawHelpModal(const Translation &translation);
    static void DrawWarningsModal(const Translation &translation);

    FontPreviewPanel m_PreviewPanel;
};
}
}