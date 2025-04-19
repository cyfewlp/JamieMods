//
// Created by jamie on 2025/4/7.
//

#include "SosGui.h"

#include "SosDataType.h"
#include "common/config.h"
#include "data/id.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"

#include <RE/A/Actor.h>
#include <Translation.h>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <cstdint>
#include <format>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    void SosGui::RenderCharactersPanel()
    {
        if (ImGuiUtil::BeginChild("$SkyOutSys_Text_ActiveActorHeader", ImVec2(), ImGuiChildFlags_AutoResizeY))
        {
            if (ImGuiUtil::CheckBox("$SkyOutSys_Text_AddActorSelection", &m_fShowNearNpc))
            {
                *this << m_dataCoordinator.RequestNearActorList();
            }
            if (m_fShowNearNpc)
            {
                ImGui::SameLine();
                RenderNearNpcList();
            }
            RenderCharactersList();
        }
        ImGui::EndChild();
    }

    void SosGui::CharactersContextMenu(const OutfitId &outfitId)
    {
        if (!ImGui::BeginPopupContextItem("##CharactersContextMenu"))
        {
            return;
        }
        if (ImGui::MenuItem("Turn to outfit list"))
        {
            m_outfitListTable.FocusOutfit(outfitId);
        }
        ImGui::EndPopup();
    }

    void SosGui::RenderCharactersList()
    {
        const auto &actors = m_uiData.GetActors();
        if (ImGuiUtil::Button("$SosGui_Refresh{$Characters}"))
        {
            m_dataCoordinator.RequestActorList();
        }
        if (!m_charactersTable.Begin())
        {
            return;
        }
        m_charactersTable.HeadersRow();
        static int selectedIdx = 0;
        int        idx         = 0;
        for (const auto &actor : actors)
        {
            ImGui::PushID(idx);

            ImGui::TableNextColumn(); // character column
            {
                constexpr auto flags      = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                const bool     isSelected = selectedIdx == idx;
                if (ImGui::Selectable(actor->GetName(), isSelected, flags))
                {
                    selectedIdx = idx;
                }
                if (isSelected)
                {
                    m_context.editingActor = actor;
                    ImGuiUtil::AddItemRectWithCol(ImGuiCol_HeaderActive, 2.5F);
                }
            }

            ImGui::TableNextColumn(); // remove character column
            if (ImGui::Button(m_charactersTable.GetHeader(1).data()))
            {
                *this << m_dataCoordinator.RequestRemoveActor(actor);
            }

            ImGui::TableNextColumn(); // active outfit column
            {
                const auto &activeOutfitMap = m_uiData.GetActorOutfitMap();
                activeOutfitMap.GetOutfit(actor);

                auto id      = activeOutfitMap.GetOutfit(actor);
                bool isFound = id != INVALID_ID;
                if (isFound)
                {
                    auto outfitOpt  = m_uiData.GetOutfitList().GetOutfit(id);
                    auto outfitName = outfitOpt
                                          .map([](const auto &outfit) {
                                              return outfit.GetName();
                                          })
                                          .value_or("No outfit");
                    ImGui::Text("%s", outfitName.c_str());
                }
                ImGui::BeginDisabled(!isFound);
                {
                    CharactersContextMenu(id);
                }
                ImGui::EndDisabled();
            }
            idx++;
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    void SosGui::RenderNearNpcList()
    {
        static int  selectedIdx = 0;
        const auto &nearActors  = m_uiData.GetNearActors();
        if (nearActors.empty())
        {
            return;
        }
        if (ImGui::BeginCombo("##ActorNearPC", nearActors.at(selectedIdx)->GetName()))
        {
            int idx = 0;
            for (const auto &nearActor : nearActors)
            {
                if (ImGui::Selectable(nearActor->GetName(), idx == selectedIdx, ImGuiSelectableFlags_None))
                {
                    selectedIdx = idx;
                    *this << m_dataCoordinator.RequestAddActor(nearActors.at(idx));
                }
                if (selectedIdx == idx)
                {
                    ImGui::SetItemDefaultFocus();
                }
                idx++;
            }
            ImGui::EndCombo();
        }
    }

    void SosGui::RenderLocationBasedAutoswitch(RE::Actor *currentActor, ImVec2 &childSize)
    {
        ImGuiUtil::ChildGuard child("##LocationAutoSwitch", childSize, ImGuiChildFlags_AutoResizeY);
        if (currentActor == nullptr)
        {
            ImGui::Text("Please select a actor to edit");
            return;
        }

        bool fAutoSwitchEnabled = m_uiData.IsAutoSwitchEnabled(currentActor);
        if (ImGuiUtil::CheckBox("$SkyOutSys_MCMHeader_Autoswitch", &fAutoSwitchEnabled))
        {
            *this << m_dataCoordinator.RequestSetActorAutoSwitchState(currentActor, fAutoSwitchEnabled);
        }

        if (!fAutoSwitchEnabled)
        {
            return;
        }
        // don't call GetAutoSwitchStateArray because it's result is static
        static constexpr std::array stateArray = {StateType::Combat,      StateType::World,   StateType::WorldSnowy,
                                                  StateType::WorldRainy,  StateType::City,    StateType::CitySnowy,
                                                  StateType::CityRainy,   StateType::Town,    StateType::TownSnowy,
                                                  StateType::TownRainy,   StateType::Dungeon, StateType::DungeonSnowy,
                                                  StateType::DungeonRainy};
        if (!m_locationAutoSwitchTable.Begin())
        {
            return;
        }
        m_locationAutoSwitchTable.HeadersRow();
        for (const auto &state : stateArray)
        {
            auto stateV = static_cast<uint32_t>(state);
            ImGui::PushID(stateV);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            auto key        = std::format("$SkyOutSys_Text_Autoswitch{}", stateV);
            bool isSelected = state == m_context.editingState;
            auto flags      = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;
            if (ImGuiUtil::Selectable(key.c_str(), isSelected, flags))
            {
                if (!isSelected)
                {
                    *this << m_outfitService.GetActorStateOutfit(currentActor, state);
                }
                m_context.editingState = isSelected ? StateType::None : state;
            }

            ImGui::TableNextColumn();
            ComboStateOutfitList(state);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    void SosGui::ComboStateOutfitList(const StateType &state)
    {
        static auto NONE_STATE = Translation::Translate("$SkyOutSys_AutoswitchEdit_None");

        auto &outfitName = m_uiData.GetActorOutfitByState(m_context.editingActor, state);
        auto  flags      = ImGuiComboFlags_HeightRegular;
        auto &preview    = outfitName.empty() ? NONE_STATE : outfitName;
        if (!ImGui::BeginCombo("##OutfitListCombo", preview.c_str(), flags))
        {
            return;
        }

        if (ImGuiUtil::Selectable("$SkyOutSys_AutoswitchEdit_None", outfitName.empty()))
        {
            if (!outfitName.empty())
            {
                *this << m_outfitService.SetActorStateOutfit(m_context.editingActor, state, "");
            }
        }

        m_uiData.GetOutfitList().for_each([&](const auto &outfit, size_t index) {
            ImGui::PushID(index);
            bool isSelected = outfitName == outfit.GetName();
            if (ImGui::Selectable(outfit.GetName().c_str(), isSelected))
            {
                if (!isSelected)
                {
                    *this << m_outfitService.SetActorStateOutfit(m_context.editingActor, state, outfit.GetName());
                }
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
        });
        ImGui::EndCombo();
    }
}