//
// Created by jamie on 2026/1/15.
//

#pragma once

#include "common/config.h"
#include "core/Translation.h"
#include "ui/Settings.h"
#include "ui/fonts/FontBuilder.h"

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
    explicit FontBuilderView(Translation &translation) : m_translation(translation) {}

    void Draw(FontBuilder &fontBuilder, Settings &settings);

private:
    bool DrawFontViewer(FontBuilder &fontBuilder, const Settings &settings);
    void DrawHelpModal() const;
    void DrawWarningsModal() const;

    ImGuiTextFilter m_filter = {};
    Translation    &m_translation;
};
}
}