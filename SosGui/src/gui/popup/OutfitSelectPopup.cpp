//
// Created by jamie on 2025/5/14.
//
#include "OutfitSelectPopup.h"

#include "data/OutfitList.h"
#include "gui/icon.h"
#include "imguiex/imguiex_m3.h"

namespace SosGui
{
void OutfitSelectPopup::OutfitDebounceInput::Clear()
{
    DebounceInput::Clear();
    viewData.clear();
}

void OutfitSelectPopup::OutfitDebounceInput::OnInput()
{
    DebounceInput::OnInput();
    viewData.clear();
}

void OutfitSelectPopup::OutfitDebounceInput::UpdateView(const OutfitList &outfitList)
{
    dirty = false;
    viewData.clear();
    outfitList.for_each([&](const auto &outfit, size_t) {
        if (filter.PassFilter(outfit.GetName().c_str()))
        {
            viewData.push_back(&outfit);
        }
    });
}

bool OutfitSelectPopup::Draw(const char *nameKey, const OutfitList &outfitList, OutfitId &selectId)
{
    selectId = INVALID_OUTFIT_ID;
    if (!ImGui::BeginPopup(nameKey))
    {
        return false;
    }
    ImGui::BeginChild("##ChildRegion", ImVec2(0, 250), ImGuiChildFlags_AutoResizeX);

    ImGui::AlignTextToFramePadding();
    ImGuiUtil::IconButton(ICON_SEARCH);
    ImGui::SameLine(0.F, 0.F);
    if (debounceInput.Draw("##filter", "filter outfit"))
    {
        debounceInput.UpdateView(outfitList);
    }

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(debounceInput.viewData.size()));
    while (clipper.Step())
    {
        if (!(0 <= clipper.DisplayStart && clipper.DisplayStart <= clipper.DisplayEnd))
        {
            continue;
        }
        for (size_t index = static_cast<size_t>(clipper.DisplayStart); index < static_cast<size_t>(clipper.DisplayEnd); ++index)
        {
            const auto &outfit = *debounceInput.viewData[index];
            ImGui::PushID(static_cast<int>(index));
            if (ImGui::Selectable(outfit.GetName().c_str(), false))
            {
                selectId = outfit.GetId();
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }
    }

    if (selectId != INVALID_OUTFIT_ID)
    {
        debounceInput.Clear();
    }
    ImGui::EndChild();
    ImGui::EndPopup();
    return true;
}
} // namespace SosGui
