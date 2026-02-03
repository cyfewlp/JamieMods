//
// Created by jamie on 2026/1/21.
//

#pragma once

#include "ImGuiEx.h"
#include "imgui.h"

#include <string_view>

namespace ImGuiEx
{
static auto BeginRightAlign(std::string_view id) -> bool
{
    if (ImGui::BeginTable(id.data(), 2, TableFlags().SizingStretchProp()))
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
} // namespace ImGuiEx