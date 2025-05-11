//
// Created by jamie on 2025/4/7.
//

#include "SosGui.h"

#include "common/config.h"
#include "common/imgui/ImGuiScop.h"
#include "data/AutoSwitchPolicyView.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/Table.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"

#include <RE/A/Actor.h>
#include <RE/B/BSCoreTypes.h>
#include <Translation.h>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <format>
#include <functional>
#include <specstrings.h>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
void SosGui::RenderCharactersPanel()
{
    if (ImGuiUtil::BeginChild("$SkyOutSys_Text_ActiveActorHeader", ImVec2(), ImGuiChildFlags_AutoResizeY))
    {
        ImGuiUtil::Text("$SkyOutSys_Text_AddActorSelection");
        ImGui::SameLine();
        if (widgets::DrawNearActorsCombo(m_uiData.GetNearActors(), &m_selectedActor,
                                         RE::PlayerCharacter::GetSingleton()))
        {
            +[&] {
                return m_dataCoordinator.RequestAddActor(m_selectedActor);
            };
        }
        RenderCharactersList();
    }
    ImGui::EndChild();
}

static auto get_actor_outfit_name(SosUiData &uiData, RE::Actor *actor) -> std::string
{
    return uiData.GetActorOutfitMap()
                 .TryGetOutfitId(actor)
                 .flat_map([&](auto &id) {
                     return uiData.GetOutfitList().GetOutfitById(id);
                 })
                 .map([](const auto &outfit) {
                     return outfit.GetName();
                 })
                 .value_or("No outfit");
}

void SosGui::RenderCharactersList()
{
    const auto &actors = m_uiData.GetActors();

    if (ImGuiUtil::Button("$SosGui_Refresh"))
    {
        +[&] {
            return m_dataCoordinator.RequestActorList();
        };
    }
    ImGuiScope::Table charactersTable("##CharactersTable", 3, TableFlags().Resizable().SizingStretchProp().flags);
    if (!charactersTable)
    {
        return;
    }
    TableHeadersBuilder()
        .Column("$Characters")
        .Column("$Delete")
        .Column("$SosGui_TableHeader_ActiveOutfit")
        .CommitHeadersRow();
    std::function<void()> onDeleteActor = [] {};
    int                   idx           = 0;
    for (const auto &actor : actors)
    {
        ImGuiScope::PushId pushId(idx);

        ImGui::TableNextColumn(); // character column
        {
            const bool isSelected = m_selectedActorIndex == idx;
            if (constexpr auto flags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                ImGui::Selectable(actor->GetName(), isSelected, flags))
            {
                m_selectedActorIndex = idx;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::TableNextColumn(); // remove character column
        if (ImGuiUtil::Button("$Delete"))
        {
            onDeleteActor = [&, actor] {
                +[&, actor] {
                    return m_dataCoordinator.RequestRemoveActor(actor);
                };
            };
        }

        ImGui::TableNextColumn(); // active outfit column
        {
            auto outfitName = get_actor_outfit_name(m_uiData, actor);
            if (ImGui::Selectable(outfitName.c_str(), false))
            {
                m_selectedActorIndex = idx;
                m_outfitSelectPopup.Open();
            }
        }
        idx++;
    }
    if (actors.empty()) return;

    onDeleteActor();

    static int prevSelected  = -1;
    const auto selectedActor = actors.at(m_selectedActorIndex);
    if (prevSelected != m_selectedActorIndex)
    {
        m_outfitListTable.OnSelectActor(selectedActor);
    }
    prevSelected = m_selectedActorIndex;

    OutfitId    selectId   = INVALID_OUTFIT_ID;
    const auto &outfitList = m_uiData.GetOutfitList();
    if (m_outfitSelectPopup.draw("Outfit List##OutfitSelectList", outfitList, selectId); selectId != INVALID_OUTFIT_ID)
    {
        if (auto opt = outfitList.GetOutfitById(selectId); opt.has_value())
        {
            +[&] {
                return m_outfitService.SetActorOutfit(selectedActor, opt.value().GetId(), opt.value().GetName());
            };
        }
    }
}

void SosGui::AutoSwitchPoliesTable(RE::Actor *currentActor)
{
    ImGuiScope::Child child("##LocationAutoSwitch", {0, 0}, ImGuiChildFlags_AutoResizeY);

    if (currentActor == nullptr)
    {
        return;
    }

    bool fAutoSwitchEnabled = m_uiData.IsAutoSwitchEnabled(currentActor->GetFormID());
    if (ImGuiUtil::CheckBox("$SkyOutSys_MCMHeader_Autoswitch", &fAutoSwitchEnabled))
    {
        +[&] {
            return m_dataCoordinator.RequestSetActorAutoSwitchState(currentActor, fAutoSwitchEnabled);
        };
    }

    if (!fAutoSwitchEnabled)
    {
        return;
    }
    if (ImGuiScope::Table table("##AutoSwitchStateList", 2,
                                TableFlags().Resizable().SizingStretchProp().RowBg().Borders().flags); table)
    {
        TableHeadersBuilder()
            .Column("$SosGui_TableHeader_Location")
            .Column("$SosGui_TableHeader_Location_State")
            .CommitHeadersRow();
        using Policy = AutoSwitchPolicyView::Policy;
        for (uint32_t policyId = 0; policyId < static_cast<uint32_t>(Policy::Count); ++policyId)
        {
            ImGuiScope::PushId pushId(policyId);
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn())
            {
                auto       key        = std::format("$SkyOutSys_Text_Autoswitch{}", policyId);
                const bool isSelected = m_autoSwitchOutfitSelectPopup.selectPolicyId == policyId;
                if (constexpr auto flags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                    ImGuiUtil::Selectable(key.c_str(), isSelected, flags))
                {
                    m_autoSwitchOutfitSelectPopup.selectPolicyId = policyId;
                    m_autoSwitchOutfitSelectPopup.Open();
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            autoSwitch_column1_outfit(currentActor->GetFormID(), policyId);
        }
    }

    OutfitId selectId = INVALID_OUTFIT_ID;
    if (m_autoSwitchOutfitSelectPopup.draw("Outfit List##AutoSwitch", m_uiData.GetOutfitList(),
                                           selectId); selectId != INVALID_OUTFIT_ID)
    {
        +[&, selectId] {
            return m_outfitService.SetActorStateOutfit(currentActor, m_autoSwitchOutfitSelectPopup.selectPolicyId,
                                                       selectId);
        };
        m_autoSwitchOutfitSelectPopup.selectPolicyId = -1;
    }
}

void SosGui::autoSwitch_column1_outfit(const RE::FormID actorId, const uint32_t policyId)
{
    const auto &view       = m_uiData.GetAutoSwitchPolicyView();
    auto &      outfitList = m_uiData.GetOutfitList();

    if (!ImGui::TableNextColumn())
    {
        return;
    }
    const auto name = view.TryFind(actorId, policyId)
                          .map([](auto &it) {
                              return it->outfitId;
                          })
                          .flat_map([&](auto &outfitId) {
                              return outfitList.GetOutfitById(outfitId);
                          })
                          .map(GetOutfitName)
                          .value_or_eval([] {
                              return Translation::Translate("$SkyOutSys_AutoswitchEdit_None");
                          });
    ImGui::Text("%s", name.c_str());
}

void SosGui::outfit_select_popup::draw(const char *nameKey, const OutfitList &outfitList, __out OutfitId &selectId)
{
    selectId = INVALID_OUTFIT_ID;
    if (!preDraw(nameKey))
    {
        return;
    }
    ImGui::BeginChild("##ChildRegion", ImVec2(0, 250), ImGuiChildFlags_AutoResizeX);

    if (debounceInput.Draw("##filter", "filter outfit"))
    {
        debounceInput.updateView(outfitList);
    }

    ImGuiListClipper clipper;
    clipper.Begin(debounceInput.viewData.size());
    while (clipper.Step())
    {
        for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
        {
            const auto &       outfit = *debounceInput.viewData.at(index);
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
        debounceInput.clear();
    }
    ImGui::EndChild();
    ImGui::EndPopup();
}

}