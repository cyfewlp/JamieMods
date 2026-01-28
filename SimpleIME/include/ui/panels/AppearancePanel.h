//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "common/config.h"
#include "core/Translation.h"
#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{
struct M3Styles;
}

namespace Ime
{

class AppearancePanel
{
    ImGuiEx::M3::M3Styles &m_styles;
    Translation           &m_translation;
    ImColor                m_colorInThemeBuilder;

public:
    explicit AppearancePanel(ImGuiEx::M3::M3Styles &styles, Translation &translation)
        : m_styles(styles), m_translation(translation)
    {
    }

    void Draw(bool appearing);

    void ApplyM3Theme(uint32_t seedArgb, bool darkMode);

private:
    void DrawZoomCombo() const;
    void DrawThemeBuilder();
};

}
}
