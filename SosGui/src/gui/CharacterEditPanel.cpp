//
// Created by jamie on 2025/5/25.
//

#include "gui/CharacterEditPanel.h"

#include "data/SosUiData.h"
#include "gui/icon.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "service/SosDataCoordinator.h"
#include "task.h"

namespace SosGui
{
namespace
{
constexpr auto OUTFIT_SELECT_LIST_POPUP_NAME = "Outfit List##OutfitSelectList";
}

void CharacterEditPanel::Focus()
{
    ImGui::SetWindowFocus("$SosGui_CharacterEditPanel"_T.c_str());
    BaseGui::Focus();
}

void CharacterEditPanel::DrawOutfitSelectPopup(RE::Actor *const &selectedActor, SosUiData &uiData, const OutfitService &outfitService)
{
    OutfitId    selectId   = INVALID_OUTFIT_ID;
    const auto &outfitList = uiData.GetOutfitList();

    if (m_outfitSelectPopup)
    {
        auto isHided = !m_outfitSelectPopup->Draw(OUTFIT_SELECT_LIST_POPUP_NAME, outfitList, selectId);
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

void CharacterEditPanel::Draw(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    if (!IsShowing())
    {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_MAIN_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("$SosGui_CharacterEditPanel"_T.c_str(), &m_show, ImGuiWindowFlags_NoNav))
    {
        DrawCharactersPanel(uiData, dataCoordinator, outfitService);

        if (const auto &actors = uiData.GetActors(); m_selectedActorIndex >= 0 && static_cast<size_t>(m_selectedActorIndex) < actors.size())
        {
            if (const auto &selectedActor = actors.at(m_selectedActorIndex))
            {
                m_autoSwitchOutfitView.Draw(selectedActor, uiData, dataCoordinator, outfitService);
            }
        }
    }
    ImGui::End();
}

void CharacterEditPanel::DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    if (ImGuiUtil::IconButton(ICON_REFRESH_CW))
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

    if (RE::Actor *selectedActor = nullptr; widgets::DrawNearActorsCombo(uiData.GetNearActors(), &selectedActor, RE::PlayerCharacter::GetSingleton()))
    {
        +[&] {
            return dataCoordinator.RequestAddActor(selectedActor);
        };
    }
    DrawCharactersTable(uiData, dataCoordinator, outfitService);
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

void CharacterEditPanel::DrawCharactersTable(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService)
{
    const auto &actors = uiData.GetActors();

    int wantDeleteActorIndex = -1;
    if (ImGui::BeginTable("##CharactersTable", 3, ImGuiEx::TableFlags().Resizable().SizingStretchProp()))
    {
        ImGui::TableSetupColumn("$Characters"_T.c_str());
        ImGui::TableSetupColumn("$SosGui_TableHeader_ActiveOutfit"_T.c_str());
        ImGui::TableSetupColumn("$Delete"_T.c_str());
        ImGui::TableHeadersRow();

        int idx = 0;
        for (const auto &actor : actors)
        {
            ImGui::PushID(idx);

            ImGui::TableNextColumn(); // character column
            {
                const bool isSelected = m_selectedActorIndex == idx;
                if (constexpr auto flags = ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns();
                    ImGui::Selectable(actor->GetName(), isSelected, flags))
                {
                    m_selectedActorIndex = idx;
                    ImGui::OpenPopup(OUTFIT_SELECT_LIST_POPUP_NAME);
                    m_outfitSelectPopup = std::make_unique<OutfitSelectPopup>();
                    m_outfitSelectPopup->UpdateView(uiData.GetOutfitList());
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
                DrawOutfitSelectPopup(actor, uiData, outfitService);
            }

            ImGui::TableNextColumn(); // active outfit column
            {
                auto outfitName = GetActorOutfitName(uiData, actor);
                ImGuiUtil::Text(outfitName);
            }

            ImGui::TableNextColumn(); // remove character column
            if (ImGuiUtil::Button("$Delete"))
            {
                wantDeleteActorIndex = idx;
            }
            ImGui::PopID();
            idx++;
        }
        ImGui::EndTable();
    }
    if (actors.empty()) return;

    if (wantDeleteActorIndex >= 0)
    {
        auto actor = actors[static_cast<size_t>(wantDeleteActorIndex)];
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
} // namespace SosGui
