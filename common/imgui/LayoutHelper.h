//
// Created by jamie on 2026/1/18.
//

#pragma once

#include "common/config.h"
#include "common/imgui/Material3Styles.h"
#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class LayoutHelper
{
    // static float g_Factor = 1.0f;

public:
    /**
     * @return the pushed style count;
     */
    static auto PushButtonStyles(const Material3Styles::ButtonStyle &buttonStyle, bool roundButton = false) -> int
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, buttonStyle.spacing);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, buttonStyle.padding);
        if (roundButton)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, buttonStyle.rounding);
            return 3;
        }
        return 2;
    }
};
}
}
