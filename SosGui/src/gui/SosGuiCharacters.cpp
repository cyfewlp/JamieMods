//
// Created by jamie on 2025/4/7.
//

#include "SosGui.h"

#include "common/config.h"
#include "data/AutoSwitchPolicyView.h"
#include "data/OutfitList.h"
#include "data/id.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"

#include <RE/A/Actor.h>
#include <RE/B/BSCoreTypes.h>
#include <Translation.h>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <format>
#include <string>

namespace LIBC_NAMESPACE_DECL
{

    void SosGui::RenderCharactersPanel()
    {
        if (ImGuiUtil::BeginChild("$SkyOutSys_Text_ActiveActorHeader", ImVec2(), ImGuiChildFlags_AutoResizeY))
        {
            NearNpcCombo();
            RenderCharactersList();
        }
        ImGui::EndChild();
    }

    auto get_outfit_name(SosUiData &uiData, const OutfitId outfitId) -> boost::optional<const SosUiOutfit &>
    {
        return uiData.GetOutfitList().GetOutfit(outfitId);
    }

    auto get_actor_outfit_name(SosUiData &uiData, RE::Actor *actor) -> std::string
    {
        return uiData.GetActorOutfitMap()
            .TryGetOutfit(actor)
            .flat_map([&](auto &id) { return uiData.GetOutfitList().GetOutfit(id); })
            .map([](const auto &outfit) { return outfit.GetName(); })
            .value_or("No outfit");
    }

    void SosGui::RenderCharactersList()
    {
        const auto &actors = m_uiData.GetActors();
        if (ImGuiUtil::Button("$SosGui_Refresh{$Characters}"))
        {
            +[&] {
                return m_dataCoordinator.RequestActorList();
            };
        }
        if (constexpr auto flags = TableFlags().Resizable().SizingStretchProp().flags;
            !ImGui::BeginTable("##CharactersTable", 3, flags))
        {
            return;
        }
        TableHeadersBuilder()
            .Column("$Characters")
            .Column("$Delete")
            .Column("$SosGui_TableHeader_ActiveOutfit")
            .CommitHeadersRow();
        static ImGuiID outfitPopupId = -1;
        static int     selectedIdx   = 0;
        int            idx           = 0;
        for (const auto &actor : actors)
        {
            ImGui::PushID(idx);

            ImGui::TableNextColumn(); // character column
            {
                const bool isSelected = selectedIdx == idx;
                if (constexpr auto flags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                    ImGui::Selectable(actor->GetName(), isSelected, flags))
                {
                    selectedIdx = idx;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::TableNextColumn(); // remove character column
            if (ImGui::Button(m_charactersTable.GetHeader(1).data()))
            {
                +[&] {
                    return m_dataCoordinator.RequestRemoveActor(actor);
                };
            }

            ImGui::TableNextColumn(); // active outfit column
            {
                auto outfitName = get_actor_outfit_name(m_uiData, actor);
                if (ImGui::Selectable(outfitName.c_str(), false))
                {
                    ImGui::OpenPopup(outfitPopupId);
                }
            }
            idx++;
            ImGui::PopID();
        }
        ImGui::EndTable();
        if (actors.empty()) return;

        m_context.editingActor = actors.at(selectedIdx);
        if (const auto opt =
                outfit_select_popup(outfitPopupId).flat_map([&](auto &id) { return get_outfit_name(m_uiData, id); });
            opt.has_value())
        {
            +[&] {
                return m_outfitService.SetActorOutfit(m_context.editingActor, opt.value().GetId(),
                                                      opt.value().GetName());
            };
        }
    }

    void SosGui::NearNpcCombo() const
    {
        static int  selectedIdx = 0;
        const auto &nearActors  = m_uiData.GetNearActors();
        if (!nearActors.empty() &&
            ImGuiUtil::BeginCombo("$SkyOutSys_Text_AddActorSelection", nearActors.at(selectedIdx)->GetName()))
        {
            int idx = 0;
            for (const auto &nearActor : nearActors)
            {
                if (ImGui::Selectable(nearActor->GetName(), idx == selectedIdx))
                {
                    selectedIdx = idx;
                    +[&] {
                        return m_dataCoordinator.RequestAddActor(nearActors.at(idx));
                    };
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

    void SosGui::AutoSwitchPoliesTable(RE::Actor *currentActor)
    {
        ImGuiUtil::ChildGuard child("##LocationAutoSwitch", {0, 0}, ImGuiChildFlags_AutoResizeY);
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
        if (constexpr auto tableFLags = TableFlags().Resizable().SizingStretchProp().RowBg().Borders().flags;
            !ImGui::BeginTable("##AutoSwitchStateList", 2, tableFLags))
        {
            return;
        }
        TableHeadersBuilder()
            .Column("$SosGui_TableHeader_Location")
            .Column("$SosGui_TableHeader_Location_State")
            .CommitHeadersRow();
        static ImGuiID  outfitPopupId    = -1;
        static uint32_t selectedPolicyId = 14;
        for (uint32_t policyId = 0; policyId < 13; ++policyId)
        {
            ImGui::PushID(policyId);
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn())
            {
                auto       key        = std::format("$SkyOutSys_Text_Autoswitch{}", policyId);
                const bool isSelected = selectedPolicyId == policyId;
                if (constexpr auto flags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                    ImGuiUtil::Selectable(key.c_str(), isSelected, flags))
                {
                    selectedPolicyId = policyId;
                    ImGui::OpenPopup(outfitPopupId);
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            autoSwitch_column1_outfit(currentActor->GetFormID(), policyId);
            ImGui::PopID();
        }
        ImGui::EndTable();

        if (auto opt = outfit_select_popup(outfitPopupId); opt.has_value())
        {
            +[&] {
                return m_outfitService.SetActorStateOutfit(currentActor, selectedPolicyId, opt.value());
            };
        }
    }

    void SosGui::autoSwitch_column1_outfit(const RE::FormID actorId, const uint32_t policyId)
    {
        const auto &view       = m_uiData.GetAutoSwitchPolicyView();
        const auto &outfitList = m_uiData.GetOutfitList();
        if (!ImGui::TableNextColumn())
        {
            return;
        }
        const auto name = view.TryFind(actorId, policyId)
                              .flat_map([&](auto &it) { return outfitList.GetOutfit(it->outfitId); })
                              .map([](auto &outfit) { return outfit.GetName(); })
                              .value_or_eval([] { return Translation::Translate("$SkyOutSys_AutoswitchEdit_None"); });
        ImGui::Text("%s", name.c_str());
    }

    auto SosGui::outfit_select_popup(__out ImGuiID &popupId) -> boost::optional<OutfitId>
    {
        boost::optional<OutfitId> selectedOutfit = boost::none;

        popupId = ImGui::GetID("Outfit List##AutoSwitchOutfitList");
        if (!ImGui::BeginPopup("Outfit List##AutoSwitchOutfitList"))
        {
            return boost::none;
        }
        ImGui::BeginChild("##ChildRegion", ImVec2(0, 250), ImGuiChildFlags_AutoResizeX);
        static ImGuiTextFilter Filter;

        const auto &outfitList = m_uiData.GetOutfitList();
        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
        ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
        if (ImGui::InputTextWithHint("##Filter", "filter outfit", Filter.InputBuf, IM_ARRAYSIZE(Filter.InputBuf),
                                     ImGuiInputTextFlags_EscapeClearsAll))
        {
            Filter.Build();
        }
        ImGui::PopItemFlag();

        ImGuiListClipper clipper;
        clipper.Begin(outfitList.size());
        const auto &index = outfitList.RankIndex();
        while (clipper.Step())
        {
            int stepItemsCount = clipper.DisplayEnd - clipper.DisplayStart;
            for (auto itBegin = index.nth(clipper.DisplayStart); stepItemsCount > 0 && itBegin != index.end();
                 ++itBegin)
            {
                if (const auto &outfit = *itBegin; Filter.PassFilter(outfit.GetName().c_str()))
                {
                    ImGui::PushID(stepItemsCount);
                    if (ImGui::Selectable(outfit.GetName().c_str(), false))
                    {
                        selectedOutfit = outfit.GetId();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopID();
                    --stepItemsCount;
                }
            }
            if (stepItemsCount > 0)
            {
                clipper.End();
                break;
            }
        }
        ImGui::EndChild();
        ImGui::EndPopup();
        return selectedOutfit;
    }
}