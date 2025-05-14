//
// Created by jamie on 2025/5/14.
//

#include "autoswitch/ActorPolicyView.h"

#include "common/imgui/ImGuiFlags.h"
#include "common/imgui/ImGuiScop.h"
#include "data/SosUiData.h"
#include "gui/Table.h"
#include "service/SosDataCoordinator.h"

namespace LIBC_NAMESPACE_DECL
{
namespace AutoSwitch
{
void ActorPolicyView::Draw(
    const RE::Actor *currentActor, SosUiData &uiData, const SosDataCoordinator &dataCoordinator,
    const OutfitService &outfitService
)
{
    ImGuiScope::Child child("##LocationAutoSwitch", {0, 0}, ImGuiChildFlags_AutoResizeY);

    if (currentActor == nullptr)
    {
        return;
    }

    using namespace ImGuiUtil;
    bool fAutoSwitchEnabled = uiData.IsAutoSwitchEnabled(currentActor->GetFormID());
    if (CheckBox("$SkyOutSys_MCMHeader_Autoswitch", &fAutoSwitchEnabled))
    {
        +[&] {
            return dataCoordinator.RequestSetActorAutoSwitchState(currentActor, fAutoSwitchEnabled);
        };
    }

    if (!fAutoSwitchEnabled)
    {
        return;
    }
    static uint32_t selectedPolicyId = 0;
    if (const auto table = ImGuiScope::Table(
            "##AutoSwitchStateList", 2, TableFlags().Resizable().SizingStretchProp().RowBg().Borders().flags
        ))
    {
        TableHeadersBuilder()
            .Column("$SosGui_TableHeader_Location")
            .Column("$SosGui_TableHeader_Location_State")
            .CommitHeadersRow();
        using Policy = AutoSwitch::Policy;
        for (uint32_t policyId = 0; policyId < static_cast<uint32_t>(Policy::Count); ++policyId)
        {
            ImGuiScope::PushId pushId(policyId);
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn())
            {
                auto       key        = std::format("$SkyOutSys_Text_Autoswitch{}", policyId);
                const bool isSelected = selectedPolicyId == policyId;
                if (constexpr auto flags = SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                    Selectable(key.c_str(), isSelected, flags))
                {
                    selectedPolicyId = policyId;

                    outfitSelectPopup = std::make_unique<Popup>(currentActor, policyId);
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            Column1Outfit(currentActor->GetFormID(), policyId, uiData);
        }
    }

    OutfitId selectId = INVALID_OUTFIT_ID;
    if (outfitSelectPopup)
    {
        bool isHided = outfitSelectPopup->Draw("Outfit List##AutoSwitch", uiData.GetOutfitList(), selectId);
        if (selectId != INVALID_OUTFIT_ID)
        {
            +[&, selectId] {
                return outfitService.SetActorStateOutfit(
                    outfitSelectPopup->actor, outfitSelectPopup->selectPolicyId, selectId
                );
            };
        }
        if (isHided)
        {
            outfitSelectPopup = nullptr;
        }
    }
}

void ActorPolicyView::Column1Outfit(const RE::FormID actorId, const uint32_t policyId, SosUiData &uiData)
{
    const auto &view       = uiData.GetAutoSwitchPolicyContainer();
    auto       &outfitList = uiData.GetOutfitList();

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
}