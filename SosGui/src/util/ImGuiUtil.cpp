#include "util/ImGuiUtil.h"
#include "common/config.h"

namespace
LIBC_NAMESPACE_DECL
{
void ImGuiUtil::may_update_table_sort_dir(bool &ascend)
{
    if (auto *sortSpecs = ImGui::TableGetSortSpecs(); sortSpecs != nullptr)
    {
        if (sortSpecs->SpecsDirty)
        {
            const auto direction = sortSpecs->Specs[0].SortDirection;
            ascend = direction == ImGuiSortDirection_Ascending;
            sortSpecs->SpecsDirty = false;
        }
    }
}

bool ImGuiUtil::debounce_input::draw()
{
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    if (ImGui::InputTextWithHint(label.data(), hintText.data(), filter.InputBuf,
                                 IM_ARRAYSIZE(filter.InputBuf),
                                 ImGuiInputTextFlags_EscapeClearsAll))
    {
        onInput();
    }
    ImGui::PopItemFlag();
    return ImGui::IsItemDeactivatedAfterEdit() || //
           (dirty && std::chrono::system_clock::now() - prevEditTime > duration);
}

void ImGuiUtil::debounce_input::clear()
{
    dirty = true;
    filter.Clear();
}

}