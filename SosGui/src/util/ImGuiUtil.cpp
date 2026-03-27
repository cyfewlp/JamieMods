#include "util/ImGuiUtil.h"

#include "imguiex/Material3.h"
#include "imguiex/m3/facade/icon_button.h"

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

namespace
{
/**
 * FIXME: Retrieve the icon directly from the @c ImFontAtlas::Fonts collection.
 * @note **Assumed Convention:**
 * This implementation relies on the following layout:
 * 1. The Icon Font is never merged.
 * 2. The Icon Font is always the second entry in the atlas index [1].
 * 3. Fallback: If the icon font is unavailable, the default font will be used.
 */
auto GetIconFont() -> ImFont *
{
    ImFontAtlas *fontAtlas = ImGui::GetIO().Fonts;
    if (fontAtlas->Fonts.size() > 1)
    {
        return fontAtlas->Fonts[1];
    }
    return ImGui::GetFont();
}

} // namespace

auto ImGuiUtil::IconButton(std::string_view icon, const ImVec2 &size) -> bool
{
    constexpr auto iconSize = M3Spec::IconButtonSizing<ImGuiEx::M3::Spec::SizeTips::XSMALL>::IconSize;
    const auto     pixels   = ImGuiEx::M3::Context::GetM3Styles().GetPixels(iconSize);
    ImFont        *iconFont = GetIconFont();
    ImGui::PushFont(iconFont, pixels);
    const auto clicked = ImGui::Button(icon.data(), size);
    ImGui::PopFont();
    return clicked;
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
