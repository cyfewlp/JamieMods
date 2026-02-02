//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "common/config.h"
#include "imgui.h"

#include <cstdint>

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
    ImColor                m_colorInThemeBuilder;
    bool                   m_darkModeInThemeBuilder = false;

public:
    explicit AppearancePanel(ImGuiEx::M3::M3Styles &styles) : m_styles(styles) {}

    void Draw(bool appearing);

    void ApplyM3Theme();

private:
    void DrawZoomCombo() const;
    void DrawThemeBuilder();
};

}
}
