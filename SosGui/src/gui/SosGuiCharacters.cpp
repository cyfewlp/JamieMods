//
// Created by jamie on 2025/4/7.
//
#include "SosGui.h"
#include "autoswitch/ActorPolicyContainer.h"
#include "common/config.h"
#include "common/imgui/ImGuiFlags.h"
#include "common/imgui/ImGuiScope.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/Table.h"
#include "gui/icon.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"

#include <RE/A/Actor.h>
#include <RE/B/BSCoreTypes.h>
#include <Translation.h>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
void SosGui::DrawCharactersPanel()
{
    if (ImGuiUtil::BeginChild("$SkyOutSys_Text_ActiveActorHeader", ImVec2(), ImGuiChildFlags_AutoResizeY))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushFont(Context::GetInstance().GetIconFont());
        if (ImGui::Button(NF_MD_REFRESH))
        {
            +[&] {
                return m_dataCoordinator.RequestActorList();
            };
        }
        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::SetItemTooltip("%s", "$SosGui_Refresh{$Characters}"_T.c_str());

        ImGui::SameLine(0, 20);

        ImGuiUtil::Text("$SkyOutSys_Text_AddActorSelection");
        ImGui::SameLine();
        if (widgets::DrawNearActorsCombo(
                m_uiData.GetNearActors(), &m_selectedActor, RE::PlayerCharacter::GetSingleton()
            ))
        {
            +[&] {
                return m_dataCoordinator.RequestAddActor(m_selectedActor);
            };
        }

        DrawCharactersList();
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

void SosGui::DrawCharactersList()
{
    const auto &actors = m_uiData.GetActors();

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
            auto outfitName = get_actor_outfit_name(m_uiData, actor);
            if (ImGui::Selectable(outfitName.c_str(), false))
            {
                m_selectedActorIndex = idx;
                m_outfitSelectPopup  = std::make_unique<OutfitSelectPopup>();
                m_outfitSelectPopup->UpdateView(m_uiData.GetOutfitList());
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
            return m_dataCoordinator.RequestRemoveActor(actor);
        };
    }

    static int prevSelected  = -1;
    const auto selectedActor = actors.at(m_selectedActorIndex);
    if (prevSelected != m_selectedActorIndex)
    {
        m_outfitListTable.OnSelectActor(selectedActor);
    }
    prevSelected = m_selectedActorIndex;

    OutfitId    selectId   = INVALID_OUTFIT_ID;
    const auto &outfitList = m_uiData.GetOutfitList();

    if (!m_outfitSelectPopup)
    {
        return;
    }
    auto isHided = !m_outfitSelectPopup->Draw("Outfit List##OutfitSelectList", outfitList, selectId);
    if (selectId != INVALID_OUTFIT_ID)
    {
        if (const auto opt = outfitList.GetOutfitById(selectId); opt.has_value())
        {
            +[&] {
                return m_outfitService.SetActorOutfit(selectedActor, opt.value().GetId(), opt.value().GetName());
            };
            util::RefreshActorArmor(selectedActor);
        }
    }
    if (isHided)
    {
        m_outfitSelectPopup = nullptr;
    }
}

void SosGui::autoSwitch_column1_outfit(const RE::FormID actorId, const uint32_t policyId)
{
    const auto &view       = m_uiData.GetAutoSwitchPolicyContainer();
    auto       &outfitList = m_uiData.GetOutfitList();

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
}