//
// Created by jamie on 2025/5/14.
//

#include "ActorPolicyView.h"

#include "data/SosUiData.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "service/SosDataCoordinator.h"

namespace SosGui::AutoSwitch
{
void ActorPolicyView::Draw(
    const RE::Actor *currentActor, SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService
)
{
    if (!ImGui::BeginChild("##LocationAutoSwitch", {0, 0}, ImGuiChildFlags_AutoResizeY))
    {
        ImGui::EndChild();
        return;
    }

    using namespace ImGuiUtil;
    bool fAutoSwitchEnabled = uiData.IsAutoSwitchEnabled(currentActor->GetFormID());
    if (ImGui::Checkbox("$SkyOutSys_MCMHeader_Autoswitch", &fAutoSwitchEnabled))
    {
        +[&] {
            return dataCoordinator.RequestSetActorAutoSwitchState(currentActor, fAutoSwitchEnabled);
        };
    }

    if (!fAutoSwitchEnabled)
    {
        ImGui::EndChild();
        return;
    }
    static uint32_t selectedPolicyId = 0;
    if (ImGui::BeginTable("##AutoSwitchStateList", 2, ImGuiEx::TableFlags().Resizable().SizingStretchProp().RowBg().Borders()))
    {
        ImGui::TableSetupColumn("$SosGui_TableHeader_Location"_T.c_str());
        ImGui::TableSetupColumn("$SosGui_TableHeader_Location_State"_T.c_str());
        ImGui::TableHeadersRow();
        for (Policy policy :
             {Policy::Combat,
              Policy::World,
              Policy::WorldSnowy,
              Policy::WorldRainy,
              Policy::City,
              Policy::CitySnowy,
              Policy::CityRainy,
              Policy::Town,
              Policy::TownSnowy,
              Policy::TownRainy,
              Policy::Dungeon,
              Policy::DungeonSnowy,
              Policy::DungeonRainy})
        {
            uint32_t policyId = static_cast<uint32_t>(policy);

            ImGui::PushID(policyId);
            ImGui::TableNextRow();
            if (ImGui::TableNextColumn())
            {
                auto       key        = std::format("$SkyOutSys_Text_Autoswitch{}", policyId);
                const bool isSelected = selectedPolicyId == policyId;
                if (constexpr auto flags = ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns(); Selectable(key.c_str(), isSelected, flags))
                {
                    selectedPolicyId = policyId;

                    outfitSelectPopup = std::make_unique<Popup>(currentActor, policyId);
                    outfitSelectPopup->UpdateView(uiData.GetOutfitList());
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            Column1Outfit(currentActor->GetFormID(), policyId, uiData);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    OutfitId selectId = INVALID_OUTFIT_ID;
    if (outfitSelectPopup)
    {
        bool isHided = !outfitSelectPopup->Draw("Outfit List##AutoSwitch", uiData.GetOutfitList(), selectId);
        if (selectId != INVALID_OUTFIT_ID)
        {
            +[&, selectId] {
                return outfitService.SetActorStateOutfit(outfitSelectPopup->actor, outfitSelectPopup->selectPolicyId, selectId);
            };
        }
        if (isHided)
        {
            outfitSelectPopup = nullptr;
        }
    }
    ImGui::EndChild();
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

} // namespace SosGui::AutoSwitch
