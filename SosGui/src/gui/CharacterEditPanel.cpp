//
// Created by jamie on 2025/5/25.
//

#include "gui/CharacterEditPanel.h"

#include "common/imgui/ImGuiFlags.h"
#include "common/imgui/ImGuiScope.h"
#include "data/SosUiData.h"
#include "gui/Table.h"
#include "gui/icon.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "service/SosDataCoordinator.h"
#include "task.h"

namespace LIBC_NAMESPACE_DECL
{
void CharacterEditPanel::Focus()
{
    ImGui::SetWindowFocus("$SosGui_CharacterEditPanel"_T.c_str());
    BaseGui::Focus();
}

void CharacterEditPanel::DrawOutfitSelectPopup(
    RE::Actor *const &selectedActor, SosUiData &uiData, const OutfitService &outfitService
)
{
    OutfitId    selectId   = INVALID_OUTFIT_ID;
    const auto &outfitList = uiData.GetOutfitList();

    if (m_outfitSelectPopup)
    {
        auto isHided = !m_outfitSelectPopup->Draw("Outfit List##OutfitSelectList", outfitList, selectId);
        if (selectId != INVALID_OUTFIT_ID)
        {
            if (const auto opt = outfitList.GetOutfitById(selectId); opt.has_value())
            {
                +[&] {
                    return outfitService.SetActorOutfit(selectedActor, opt.value().GetId(), opt.value().GetName());
                };
                util::RefreshActorArmor(selectedActor);
            }
        }
        if (isHided)
        {
            m_outfitSelectPopup = nullptr;
        }
    }
}

void CharacterEditPanel::Draw(
    SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService
)
{
    if (!IsShowing())
    {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_MAIN_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("$SosGui_CharacterEditPanel"_T.c_str(), &m_show, ImGuiWindowFlags_NoNav))
    {
        DrawCharactersPanel(uiData, dataCoordinator);

        if (const auto &actors = uiData.GetActors();
            m_selectedActorIndex >= 0 && static_cast<size_t>(m_selectedActorIndex) < actors.size())
        {
            if (const auto &selectedActor = actors.at(m_selectedActorIndex))
            {
                DrawOutfitSelectPopup(selectedActor, uiData, outfitService);
                m_autoSwitchOutfitView.Draw(selectedActor, uiData, dataCoordinator, outfitService);
            }
        }
    }
    ImGui::End();
}

void CharacterEditPanel::DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    if (ImGui::Button(NF_MD_REFRESH))
    {
        +[&] {
            return dataCoordinator.RequestActorList();
        };
    }
    ImGui::PopStyleColor();
    ImGui::SetItemTooltip("%s", "$SosGui_Refresh{$Characters}"_T.c_str());

    ImGui::SameLine(0, 20);

    ImGuiUtil::Text("$SkyOutSys_Text_AddActorSelection");
    ImGui::SameLine();

    if (RE::Actor *selectedActor = nullptr;
        widgets::DrawNearActorsCombo(uiData.GetNearActors(), &selectedActor, RE::PlayerCharacter::GetSingleton()))
    {
        +[&] {
            return dataCoordinator.RequestAddActor(selectedActor);
        };
    }
    DrawCharactersTable(uiData, dataCoordinator);
}

static auto GetActorOutfitName(SosUiData &uiData, RE::Actor *actor) -> std::string
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

void CharacterEditPanel::DrawCharactersTable(SosUiData &uiData, const SosDataCoordinator &dataCoordinator)
{
    const auto &actors = uiData.GetActors();

    const ImGuiScope::Table charactersTable(
        "##CharactersTable", 3, ImGuiUtil::TableFlags().Resizable().SizingStretchProp().flags
    );
    if (!charactersTable)
    {
        return;
    }
    TableHeadersBuilder()
        .Column("$Characters")
        .Column("$SosGui_TableHeader_ActiveOutfit")
        .Column("$Delete")
        .CommitHeadersRow();

    int wantDeleteActorIndex = -1;
    int idx                  = 0;
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

        ImGui::TableNextColumn(); // active outfit column
        {
            auto outfitName = GetActorOutfitName(uiData, actor);
            if (ImGui::Selectable(outfitName.c_str(), false))
            {
                m_selectedActorIndex = idx;
                m_outfitSelectPopup  = std::make_unique<OutfitSelectPopup>();
                m_outfitSelectPopup->UpdateView(uiData.GetOutfitList());
            }
        }

        ImGui::TableNextColumn(); // remove character column
        if (ImGuiUtil::Button("$Delete"))
        {
            wantDeleteActorIndex = idx;
        }
        idx++;
    }
    if (actors.empty()) return;

    if (wantDeleteActorIndex != -1)
    {
        auto actor = actors[wantDeleteActorIndex];
        +[&, actor] {
            return dataCoordinator.RequestRemoveActor(actor);
        };
    }
}

inline auto CharacterEditPanel::GetSelectedActor(SosUiData &uiData) const -> RE::Actor *
{
    if (const auto &actors = uiData.GetActors(); static_cast<size_t>(m_selectedActorIndex) < actors.size())
    {
        return uiData.GetActors().at(m_selectedActorIndex);
    }
    return nullptr;
}
}
