//
// Created by jamie on 2026/1/21.
//

#pragma once

#include "common/config.h"
#include "common/imgui/Material3Styles.h"

#include <imgui.h>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx
{
struct TableLayoutInfo
{
    ImVec2          outerSize = {0, 0};
    float           innerSize = 0;
    ImGuiTableFlags flags     = ImGuiTableFlags_None;
};

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

static auto BeginRightAlign(std::string_view id) -> bool
{
    if (ImGui::BeginTable(id.data(), 2, ImGuiEx::TableFlags().SizingStretchProp()))
    {
        ImGui::TableSetupColumn("#1", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("#2", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Dummy({1.f, 1.f});

        ImGui::TableNextColumn();
        return true;
    }
    return false;
}

static void EndRightAlign()
{
    ImGui::EndTable();
}
}
}