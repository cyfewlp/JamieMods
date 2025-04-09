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
            static bool fShowNearNpcLis = false;
            if (ImGuiUtil::CheckBox("$SkyOutSys_Text_AddActorSelection", &fShowNearNpcLis))
            {
                m_dataCoordinator.RequestNearActorList();
            }
            if (fShowNearNpcLis)
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

        m_charactersTable.rows = actors.size();
        RenderTable(m_charactersTable, [&actors, this](int rowIdx) {
            auto      *actor      = actors.at(rowIdx);
            bool const isSelected = selectedIdx == rowIdx;
            if (isSelected)
            {
                auto color = ImGui::GetColorU32(ImGuiCol_HeaderActive);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, color);
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable(actor->GetName(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
            {
                selectedIdx = selectedIdx != rowIdx ? rowIdx : -1;
            }

            ImGui::TableNextColumn();
            if (ImGui::Button(m_charactersTable.headersRow[1].c_str()))
            {
                m_dataCoordinator.RequestRemoveActor(actor);
            }

            ImGui::TableNextColumn();
            const auto &activeOutfitMap = m_uiData.GetActorActiveOutfitMap();
            if (auto iter = activeOutfitMap.find(actor); iter != activeOutfitMap.end())
            {
                ImGui::Text("%s", (*iter).second.c_str());
            }
        });

        if (selectedIdx >= 0 && selectedIdx < static_cast<int>(actors.size()))
        {
            m_editingActor = actors.at(selectedIdx);
            RenderLocationBasedAutoswitch(m_editingActor);
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
                    m_dataCoordinator.RequestAddActor(nearActors.at(idx));
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

    void SosGui::RenderLocationBasedAutoswitch(RE::Actor *currentActor)
    {
        bool fAutoSwitchEnabled = m_uiData.IsAutoSwitchEnabled(currentActor);
        if (ImGuiUtil::CheckBox("$SkyOutSys_MCMHeader_Autoswitch", &fAutoSwitchEnabled))
        {
            m_dataCoordinator.RequestSetActorAutoSwitchState(currentActor, fAutoSwitchEnabled);
        }

        if (!fAutoSwitchEnabled)
        {
            return;
        }
        // don't call GetAutoSwitchStateArray because it's result is static
        static constexpr std::array stateArray = {Combat,    World,        WorldSnowy,  WorldRainy, City,
                                                  CitySnowy, CityRainy,    Town,        TownSnowy,  TownRainy,
                                                  Dungeon,   DungeonSnowy, DungeonRainy};
        m_locationAutoSwitchTable.rows         = stateArray.size();
        RenderTable(m_locationAutoSwitchTable, [](int idx) {
            auto state = stateArray.at(idx);
            ImGui::TableNextColumn();
            ImGuiUtil::Text(std::format("$SkyOutSys_Text_Autoswitch{}", static_cast<int8_t>(state)));

            ImGui::TableNextColumn();
            ImGuiUtil::Text("$SkyOutSys_AutoswitchEdit_None");
        });
    }
}