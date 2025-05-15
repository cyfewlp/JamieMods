//
// Created by jamie on 2025/5/14.
//
#include "gui/OutfitSelectPopup.h"

#include "common/imgui/ImGuiScope.h"
#include "data/OutfitList.h"
#include "gui/icon.h"

namespace LIBC_NAMESPACE_DECL
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
    if (!Begin(nameKey))
    {
        return false;
    }
    ImGui::BeginChild("##ChildRegion", ImVec2(0, 250), ImGuiChildFlags_AutoResizeX);

    ImGui::AlignTextToFramePadding();
    ImGui::Text(NF_OCT_SEARCH);
    ImGui::SameLine(0, 5);
    if (debounceInput.Draw("##filter", "filter outfit"))
    {
        debounceInput.UpdateView(outfitList);
    }

    ImGuiListClipper clipper;
    clipper.Begin(debounceInput.viewData.size());
    while (clipper.Step())
    {
        for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            const auto        &outfit = *debounceInput.viewData.at(index);
            ImGuiScope::PushId pushId(index);
            if (ImGui::Selectable(outfit.GetName().c_str(), false))
            {
                selectId = outfit.GetId();
                ImGui::CloseCurrentPopup();
            }
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
}