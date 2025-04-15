//
// Created by jamie on 2025/4/7.
//

#include "SosGui.h"

#include "ImGuiUtil.h"
#include "SosDataType.h"
#include "imgui.h"

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

    void SosGui::RenderCharactersList()
    {
        const auto &actors = m_uiData.GetActors();
        if (ImGuiUtil::Button("$SosGui_Refresh{$Characters}"))
        {
            m_dataCoordinator.RequestActorList();
        }
        static int selectedIdx = -1;
        ImGui::PushFontSize(HintFontSize());
        ImGuiUtil::Text(selectedIdx == -1 ? "$SosGui_SelectHint{$Characters}" : "");
        ImGui::PopFontSize();

        if (m_charactersTable.Begin())
        {
            m_charactersTable.HeadersRow();
            int idx = 0;
            for (const auto &actor : actors)
            {
                ImGui::PushID(idx);
                bool const isSelected = selectedIdx == idx;
                if (isSelected)
                {
                    auto color = ImGui::GetColorU32(ImGuiCol_HeaderActive);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, color);
                }

                ImGui::TableNextColumn();
                if (ImGui::Selectable(actor->GetName(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                {
                    m_context.editingActor = selectedIdx != idx ? actor : nullptr;
                    selectedIdx            = selectedIdx != idx ? idx : -1;
                }

                ImGui::TableNextColumn();
                if (ImGui::Button(m_charactersTable.GetHeader(1).data()))
                {
                    *this << m_dataCoordinator.RequestRemoveActor(actor);
                }

                ImGui::TableNextColumn();
                const auto &activeOutfitMap = m_uiData.GetActorActiveOutfitMap();
                if (auto iter = activeOutfitMap.find(actor); iter != activeOutfitMap.end())
                {
                    ImGui::Text("%s", (*iter).second.c_str());
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
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
                    *this << m_dataCoordinator.RequestActorStateOutfit(currentActor, state);
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
                *this << m_dataCoordinator.RequestSetActorStateOutfit(m_context.editingActor, state, "");
            }
        }

        int idx = 0;
        for (const auto &outfit : m_uiData.GetOutfitList())
        {
            ImGui::PushID(idx);
            bool isSelected = outfitName == outfit.second.GetName();
            if (ImGui::Selectable(outfit.second.GetName().c_str(), isSelected))
            {
                if (!isSelected)
                {
                    *this << m_dataCoordinator.RequestSetActorStateOutfit(m_context.editingActor, state,
                                                                          outfit.second.GetName());
                }
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
        }

        ImGui::EndCombo();
    }
}