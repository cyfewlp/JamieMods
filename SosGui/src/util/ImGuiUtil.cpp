#include "util/ImGuiUtil.h"

namespace SosGui
{

void ImGuiUtil::AddItemRectWithCol(const ImGuiCol colorIndex, const float thickness)
{
    auto      *drawList = ImGui::GetWindowDrawList();
    const auto color    = ImGui::GetColorU32(colorIndex);
    drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color, 0, ImDrawFlags_None, thickness);
}

void ImGuiUtil::may_update_table_sort_dir(bool &ascend)
{
    if (auto *sortSpecs = ImGui::TableGetSortSpecs(); sortSpecs != nullptr)
    {
        if (sortSpecs->SpecsDirty)
        {
            const auto direction  = sortSpecs->Specs[0].SortDirection;
            ascend                = direction == ImGuiSortDirection_Ascending;
            sortSpecs->SpecsDirty = false;
        }
    }
}

bool ImGuiUtil::DebounceInput::Draw(const char *label, const char *hintText)
{
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    if (ImGui::InputTextWithHint(label, hintText, filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll))
    {
        OnInput();
    }
    ImGui::PopItemFlag();
    return ImGui::IsItemDeactivatedAfterEdit() || //
           (dirty && std::chrono::system_clock::now() - prevEditTime > duration);
}

void ImGuiUtil::DebounceInput::Clear()
{
    dirty = true;
    filter.Clear();
}
} // namespace SosGui
