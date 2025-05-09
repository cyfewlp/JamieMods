#include "gui/OutfitEditPanel.h"
#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/Config.h"
#include "gui/Popup.h"
#include "gui/Table.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "util/ImGuiUtil.h"
#include "util/utils.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cassert>
#include <expected>
#include <format>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
void OutfitEditPanel::EditContext::Clear()
{
    armorListShowAllSlotArmors  = false;
    armorGeneratorSelectedActor = nullptr;
    dirty                       = true;
}

void OutfitEditPanel::Draw(const EditingOutfit &editingOutfit)
{
    if (!IsShowing())
    {
        return;
    }
    const std::string windowName = std::format("Editing Outfit: {}###OutfitEditor", editingOutfit.GetName());
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_OUTFIT_EDIT_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    if (ImGui::Begin(windowName.c_str(), &m_show, flags))
    {
        ImGui::SameLine();
        ImGui::BeginGroup();
        DoDraw(editingOutfit);
        ImGui::EndGroup();
    }
    ImGui::End();
}

void OutfitEditPanel::DrawOutfitTabBarView(const EditingOutfit &editingOutfit)
{
    if (ImGui::BeginTabBar("##OutfitTabBarView"))
    {
        if (ImGui::BeginTabItem(editingOutfit.GetName().c_str(), nullptr, ImGuiTabItemFlags_None))
        {
            // TODO: support untitled outfit
            DrawOutfitArmors(editingOutfit);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::DoDraw(const EditingOutfit &editingOutfit)
{
    if (m_editContext.dirty)
    {
        m_editContext.dirty = false;
        m_armorView.update_view_data(GetGenerator(), editingOutfit.GetSourceOutfit());
    }
    if (ImGui::BeginChild("##OutfitEditPanelSideBar", {200, 0}, ImGuiUtil::ChildFlag().Borders().ResizeX().flags))
    {
        DrawSideBar(editingOutfit.GetSourceOutfit());
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();
    {
        ImGui::PushID(this);
        RenderPopups(editingOutfit);
        DrawOutfitTabBarView(editingOutfit);
        DrawArmorGeneratorTabBar(editingOutfit.GetSourceOutfit());
        DrawArmorViewFilter(editingOutfit.GetSourceOutfit());
        DrawArmorView(editingOutfit, m_armorView.viewData);
        ImGui::PopID();
    }
    ImGui::EndGroup();
}

void OutfitEditPanel::DrawArmorInfo()
{
    if (m_armorView.multiSelection.Size == 1)
    {
        void *  it = nullptr;
        ImGuiID selectedRank; // must be name rank;
        m_armorView.multiSelection.GetNextSelectedItem(&it, &selectedRank);
        const auto *armor = m_armorView.viewData.at(selectedRank);
        ImGuiUtil::TextScale("Armor Info", Config::FONT_SIZE_TITLE_3);
        {
            ImGui::Indent();
            ImGui::BeginGroup();
            {
                ImGui::Text("%s", armor->GetName());
                ImGuiUtil::Value("FormID", std::format("{:#010X}", armor->GetFormID()).c_str());
                ImGui::Text("%s", util::GetFormModFileName(armor).data());
                ImGuiUtil::Value("$Armor Rating", static_cast<float>(armor->armorRating) / static_cast<float>(100.0));
                ImGuiUtil::Value("$WEIGHT", armor->GetWeight());
                ImGui::Text("%sPlayable", IsArmorNonPlayable(armor) ? "Non" : "");
                ImGui::BeginGroup();
                for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; ++slotPos)
                {
                    if (armor->HasPartOf(ToSlot(slotPos)))
                    {
                        ImGuiUtil::Button(Translation::Translate(get_slot_name_key(slotPos)).c_str());
                    }
                }
                ImGui::EndGroup();
            }
            ImGui::EndGroup();
        }
    }
}

void OutfitEditPanel::DrawSideBar(const SosUiOutfit *editingOutfit)
{
    ImGuiUtil::TextScale("$SosGui_ModList", Config::FONT_SIZE_TITLE_3);
    static int maxChildItemCount = 10;
    auto       itemHeight        = ImGui::GetTextLineHeight();
    float      childHeight       = (itemHeight + ImGui::GetStyle().ItemInnerSpacing.y) * maxChildItemCount;

    if (ImGui::BeginChild("##ModNameListChild", {0, childHeight}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
    {
        DrawArmorViewModNameFilterer(editingOutfit);
    }
    ImGui::EndChild();

    ImGuiUtil::TextScale("$SosGui_BodySlots", Config::FONT_SIZE_TITLE_3);
    if (ImGui::BeginChild("#SlotFilterChild", {0, childHeight}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
    {
        DrawArmorViewSlotFilterer(editingOutfit);
    }
    ImGui::EndChild();
    DrawArmorInfo();
}

void OutfitEditPanel::Refresh() {}

void OutfitEditPanel::Close()
{
    m_editContext.Clear();
    m_armorView.clear();
}

void OutfitEditPanel::OnSelectActor(const RE::Actor *, const EditingOutfit &editingOutfit)
{
    m_armorView.update_view_data(GetGenerator(), editingOutfit.GetSourceOutfit());
}

void OutfitEditPanel::OnSelectOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editing)
{
    UpdateWindowTitle(editing.GetName());
    if (dynamic_cast<BasicArmorGenerator *>(GetGenerator()) != nullptr)
    {
        if (lastEdit.GetId() != editing.GetId())
        {
            m_armorView.add_armors_in_outfit(m_uiData, lastEdit.GetSourceOutfit());
            m_armorView.remove_armors_in_outfit(editing.GetSourceOutfit());
            m_armorView.multiSelection.Clear();
        }
    }
    else
    {
        m_armorView.update_view_data(GetGenerator(), editing.GetSourceOutfit());
    }
}

void OutfitEditPanel::UpdateWindowTitle(const std::string &outfitName)
{
    m_windowTitle = Translation::Translate("$SosGui_WindowName_EditingOutfit");
    if (const auto pos = m_windowTitle.find("{}"); pos != std::string::npos)
    {
        m_windowTitle.replace(pos, 2, outfitName);
    }
}

void OutfitEditPanel::DrawOutfitArmors(const EditingOutfit &editingOutfit)
{
    if (editingOutfit.IsUntitled())
    {
        ImGuiUtil::TextScale("$SosGui_SelectHint{$SosGui_Outfit}", Config::FONT_SIZE_TITLE_3);
        return;
    }
    if (editingOutfit.IsEmpty())
    {
        ImGuiUtil::TextScale("$SosGui_EmptyHint{$ARMOR}", Config::FONT_SIZE_TITLE_3);
        return;
    }

    ImGuiUtil::CheckBox("$SosGui_CheckBox_SlotFilter", &m_editContext.armorListShowAllSlotArmors);
    ImGui::SameLine();
    if (ImGuiUtil::Button("$SkyOutSys_OEdit_SlotPolicyHelp"))
    {
        m_slotPolicyHelp.Open();
    }

    static int selectedIdx = -1;
    if (constexpr auto flags = TableFlags().Resizable().SizingStretchProp().flags;
        ImGui::BeginTable("##OutfitArmors", 5, flags))
    {
        // clang-format off
        TableHeadersBuilder().Column("##Number").NoHide()
        .Column("$SosGui_TableHeader_Slot").NoSort()
        .Column("$ARMOR").NoSort()
        .Column("$SkyOutSys_OEdit_OutfitSettings_Header").WidthFixed().NoSort()
        .Column("$Delete").WidthFixed().NoSort()
        .CommitHeadersRow();
        // clang-format on

        for (uint32_t slotIdx = 0; slotIdx < RE::BIPED_OBJECT::kEditorTotal; ++slotIdx)
        {
            const auto *armor = editingOutfit.GetArmorAt(slotIdx);
            if (!m_editContext.armorListShowAllSlotArmors && armor == nullptr)
            {
                continue;
            }

            ImGuiUtil::PushIdGuard idHolder(slotIdx);

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%d", slotIdx + 1);

            if (ImGui::TableNextColumn()) // slot name column
            {
                const bool isSelected = static_cast<uint32_t>(selectedIdx) == slotIdx;
                auto       name       = std::format("$SkyOutSys_BodySlot{}", slotIdx + SOS_SLOT_OFFSET);
                if (constexpr auto selectableFlags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                    ImGuiUtil::Selectable(name, isSelected, selectableFlags))
                {
                    selectedIdx = isSelected ? -1 : slotIdx;
                }
            }
            if (armor != nullptr)
            {
                HighlightConflictSlot(ToSlot(slotIdx));
            }

            ImGui::TableNextColumn(); // armor name column
            ImGui::Text("%s", armor == nullptr ? "" : armor->GetName());

            ImGui::TableNextColumn(); // slot policy combo column
            SlotPolicyCombo(editingOutfit, slotIdx);

            ImGui::TableNextColumn(); // action column
            ImGui::BeginDisabled(armor == nullptr);
            {
                if (ImGuiUtil::Button("$Delete"))
                {
                    if (editingOutfit.IsUntitled())
                    {
                        m_DeleteArmorPopup.wantDeleteArmor = armor;
                        m_DeleteArmorPopup.Open();
                    }
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndTable();
    }
}

void OutfitEditPanel::HighlightConflictSlot(const Slot slot) const
{
    if (m_armorView.multiSelection.IsSelectSlot(slot))
    {
        auto *drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 0, 0, 255), 0,
                          ImDrawFlags_None, 2.0F);
    }
}

void OutfitEditPanel::SlotPolicyCombo(const EditingOutfit &editingOutfit, const uint32_t &slotIdx) const
{
    auto &      policyNameKey = editingOutfit.GetSlotPolicies().at(slotIdx);
    std::string policyName;

    if (ImGui::BeginCombo("##SlotPolicy", Translation::Translate(policyNameKey).c_str(),
                          ImGuiComboFlags_WidthFitPreview))
    {
        for (const auto &policy :
             {SlotPolicy::Inherit, SlotPolicy::Passthrough, SlotPolicy::RequireEquipped, SlotPolicy::AlwaysUseOutfit})
        {
            policyName = SlotPolicyToUiString(policy);
            if (ImGui::Selectable(policyName.c_str(), false))
            {
                if (!editingOutfit.IsUntitled())
                {
                    +[&] {
                        return m_outfitService.SetSlotPolicy(editingOutfit.GetId(), editingOutfit.GetName(), slotIdx,
                                                             policy);
                    };
                }
            }
            ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
        }
        ImGui::EndCombo();
    }
}

inline auto OutfitEditPanel::get_slot_name_key(const uint32_t slotPos) -> std::string
{
    return std::format("$SkyOutSys_BodySlot{}", slotPos + SOS_SLOT_OFFSET);
}

void OutfitEditPanel::DrawArmorGeneratorTabBar(const SosUiOutfit *editingOutfit)
{
    using namespace ImGuiUtil;
    if (ImGui::BeginTabBar("ArmorGeneratorTabBar"), TabBarFlags().DrawSelectedOverline().Reorderable().flags)
    {
        const auto *tabBar          = ImGui::GetCurrentTabBar();
        const auto  nextTabId       = tabBar->NextSelectedTabId;
        const auto  selectedId      = tabBar->SelectedTabId;
        auto        isTabItemAppear = [&] {
            const ImGuiTabItem *tabItem = ImGui::TabBarGetCurrentTab(ImGui::GetCurrentTabBar());
            return selectedId == 0 || (selectedId != nextTabId && tabItem->ID == nextTabId);
        };
        auto * player        = RE::PlayerCharacter::GetSingleton();
        auto *&selectedActor = m_armorGeneratorTabBar.selectedActor;
        if (ImGui::BeginTabItem("From Actor Inventory"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = std::make_unique<InventoryArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            Text("$SosGui_Actor_ArmorSource");
            ImGui::SameLine();
            if (widgets::DrawNearActorsCombo(m_uiData.GetNearActors(), &selectedActor, player))
            {
                m_armorGeneratorTabBar.generator = std::make_unique<InventoryArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("From Actor Carried Items"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            Text("$SosGui_Actor_ArmorSource");
            ImGui::SameLine();
            if (widgets::DrawNearActorsCombo(m_uiData.GetNearActors(), &selectedActor, player))
            {
                m_armorGeneratorTabBar.generator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("From form-id"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = nullptr;
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::Text("0x");
            ImGui::SameLine();
            static std::array<char, 9> formIdBuf;
            if (ImGui::InputText("##FormIdInput", formIdBuf.data(), formIdBuf.size(),
                                 ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal))
            {
                m_armorView.clearViewData();
                char *         pEnd{};
                const uint32_t result = strtoul(formIdBuf.data(), &pEnd, 16);
                if (pEnd != formIdBuf.data())
                {
                    m_armorGeneratorTabBar.generator = std::make_unique<FormIdArmorGenerator>(result);
                    m_armorView.reset_view(GetGenerator(), editingOutfit);
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("All Armors"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = std::make_unique<BasicArmorGenerator>();
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::DrawArmorViewTableContent(const std::vector<const Armor *> &                           viewData,
                                                const std::function<void(const Armor *armor, size_t index)> &drawAction)
{
    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);
    ImGuiListClipper clipper;
    // clang-format off
    auto *msIO = m_armorView.multiSelection
                 .NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid()
                 .Begin(viewData.size());
    clipper.Begin(viewData.size());
    m_armorView.multiSelection.ApplyRequests(msIO);
    if (msIO->RangeSrcItem != -1)
        clipper.IncludeItemByIndex(static_cast<int>(msIO->RangeSrcItem));

    // clang-format on
    while (clipper.Step())
    {
        if (ascend)
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
            {
                drawAction(viewData.at(index), index);
            }
        }
        else
        {
            using namespace std::views;
            size_t index = static_cast<size_t>(clipper.DisplayStart);
            for (int         count = clipper.DisplayEnd - clipper.DisplayStart;
                 const auto &armor : viewData | reverse | drop(clipper.DisplayStart) | take(count))
            {
                drawAction(armor, index++);
            }
        }
    }
    m_armorView.multiSelection.ApplyRequests(ImGui::EndMultiSelect());
}

void OutfitEditPanel::DrawArmorViewFilter(const SosUiOutfit *editingOutfit)
{
    // filter armor name and mod name
    if (m_armorView.armorFilter.Draw())
    {
        m_armorView.armorFilter.dirty = false;
        m_armorView.update_view_data(GetGenerator(), editingOutfit);
    }
}

void OutfitEditPanel::DrawArmorView(const EditingOutfit &editingOutfit, const std::vector<const Armor *> &viewData)
{
    if (constexpr auto flags = TableFlags().RowBg().BordersInnerH().ScrollY().Resizable().SizingFixedFit()
                                           .Sortable().Hideable().Reorderable().flags;
        viewData.empty() || !ImGui::BeginTable("##ArmorCandidates", 6, flags))
    {
        return;
    }

    ImGui::TableSetupScrollFreeze(1, 1);
    // clang-format off
    TableHeadersBuilder().Column("##Number").NoSort()
        .Column("$ARMOR").DefaultSort().NoHide()
        .Column("FormID").NoSort()
        .Column("ModName").NoSort()
        .Column("Playable").NoSort()
        .Column("$Add").WidthFixed().NoSort()
        .CommitHeadersRow();
    // clang-format on

    std::function<void()> onRequireAddArmor = [] {};

    auto drawAction = [&](const Armor *armor, const size_t index) {
        ImGuiUtil::PushIdGuard idGuard(index);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); // number column
        {
            const bool isSelected = m_armorView.multiSelection.UpdateSelected(armor, index);
            ImGui::SetNextItemSelectionUserData(index);
            constexpr auto flags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
            ImGui::Selectable(std::format("{}", index + 1).c_str(), isSelected, flags);

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGuiUtil::MenuItem("$SosGui_ContextMenu_AddAllArmor"))
                {
                    if (m_armorView.multiSelection.Size == 1)
                    {
                        onRequireAddArmor = [&, armor] {
                            OnAcceptAddArmorToOutfit(editingOutfit, armor);
                        };
                    }
                    else if (m_armorView.multiSelection.Size > 1)
                    {
                        onRequireAddArmor = [&] {
                            m_batchAddArmorsPopUp.Open();
                        };
                    }
                }
                ImGui::EndPopup();
            }
        }

        if (ImGui::TableNextColumn()) // armor name column
        {
            ImGui::Text("%s", armor->GetName());
        }

        if (ImGui::TableNextColumn()) // formId column
        {
            ImGui::Text("%s", std::format("{:#010X}", armor->GetFormID()).c_str());
        }

        if (ImGui::TableNextColumn()) // mod name column
        {
            ImGui::Text("%s", util::GetFormModFileName(armor).data());
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                ImGui::SetItemTooltip("%s", util::GetFormModFileName(armor).data());
            }
        }

        if (ImGui::TableNextColumn()) // column playable
        {
            ImGui::Text("%s", IsArmorNonPlayable(armor) ? "\xe2\x9d\x8c" : "\xe2\x9c\x85");
        }

        if (ImGui::TableNextColumn() && ImGuiUtil::Button("$Add")) // column Action
        {
            onRequireAddArmor = [&, armor] {
                OnAcceptAddArmorToOutfit(editingOutfit, armor);
            };
        }
    };

    DrawArmorViewTableContent(viewData, drawAction);
    ImGui::EndTable();
    if (!editingOutfit.IsUntitled())
    {
        onRequireAddArmor();
    }
}

void OutfitEditPanel::DrawArmorViewModNameFilterer(const SosUiOutfit *editingOutfit)
{
    bool needResetView = false;
    for (auto &modState : m_armorView.modFilterer.passModList)
    {
        if (auto &modName = modState.first; m_armorView.modRefCounter.contains(modName))
        {
            auto label = std::format("({}) {}", m_armorView.modRefCounter[modName], modName);

            bool tIsChecked = modState.second;
            if (ImGui::Checkbox(label.c_str(), &tIsChecked))
            {
                std::pair newState(modName, tIsChecked);
                modState.swap(newState);
                needResetView = true;
            }
        }
    }
    if (needResetView)
    {
        m_armorView.update_view_data(GetGenerator(), editingOutfit);
    }
}

void OutfitEditPanel::DrawArmorViewSlotFilterer(const SosUiOutfit *editing)
{
    std::string name = std::format("All({}/{})##AllSlot", m_armorView.viewData.size(), m_armorView.availableArmorCount);
    if (ImGui::Checkbox(name.c_str(), &m_armorView.checkAllSlot))
    {
        if (!m_armorView.checkAllSlot)
        {
            if (m_armorView.slotFiltererSelected == 0)
            {
                m_armorView.clearViewData();
            }
            else
            {
                auto slotValue = m_armorView.slotFiltererSelected.to_ulong();
                m_armorView.remove_armors_has_slot(static_cast<Slot>(slotValue), static_cast<Slot>(~slotValue));
            }
        }
        else
        {
            m_armorView.update_view_data(GetGenerator(), editing);
        }
    }

    for (uint8_t idx = 0; idx < RE::BIPED_OBJECT::kEditorTotal; ++idx)
    {
        ImGuiUtil::PushIdGuard idGuard(idx);

        auto slotLabel = std::format("({}) {}", m_armorView.slotCounter[idx],
                                     Translation::Translate(get_slot_name_key(idx)));
        bool checked = m_armorView.slotFiltererSelected.test(idx);
        if (ImGui::Checkbox(slotLabel.c_str(), &checked))
        {
            m_armorView.slotFiltererSelected.set(idx, checked);
            if (checked)
            {
                if (m_armorView.checkAllSlot)
                {
                    m_armorView.clearViewData();
                    m_armorView.checkAllSlot = false;
                }
                m_armorView.update_view_data(GetGenerator(), editing);
            }
            else if (!m_armorView.checkAllSlot)
            {
                if (m_armorView.slotFiltererSelected == 0)
                {
                    m_armorView.clearViewData();
                }
                else
                {
                    m_armorView.remove_armors_has_slot(static_cast<Slot>(m_armorView.slotFiltererSelected.to_ulong()),
                                                       static_cast<Slot>(1 << idx));
                }
            }
        }
    }
}

void OutfitEditPanel::BatchAddArmors(const EditingOutfit &editingOutfit)
{
    SlotEnumeration usedSlot;
    // We will remove all used armors in view;
    auto                       prevArmors = editingOutfit.GetSourceOutfit()->GetUniqueArmors();
    std::vector<const Armor *> newViewData;
    newViewData.reserve(m_armorView.viewData.size());
    for (size_t index = 0; index < m_armorView.viewData.size(); ++index)
    {
        auto *armor = m_armorView.viewData.at(index);
        if (!m_armorView.multiSelection.Contains(index) || usedSlot.all(armor->GetSlotMask()))
        {
            newViewData.emplace_back(armor);
            continue;
        }
        usedSlot.set(armor->GetSlotMask());
        AddArmorToOutfit(editingOutfit, armor);
    }
    m_armorView.multiSelection.Clear();
    m_armorView.viewData.swap(newViewData);

    const auto &armors = editingOutfit.GetSourceOutfit()->GetUniqueArmors();
    std::erase_if(prevArmors, [&armors](const Armor *armor) {
        return armors.contains(armor);
    });
    for (const auto &armor : prevArmors)
    {
        if (auto result = m_armorView.add_armor(armor); !result.has_value())
        {
            m_uiData.PushErrorMessage(
                std::format("Can't restore armor {} from outfit {}", armor->GetName(), editingOutfit.GetName()));
        }
    }
}

void OutfitEditPanel::AddArmorToOutfit(const EditingOutfit &editingOutfit, const Armor *armor) const
{
    if (editingOutfit.IsConflictWith(armor))
    {
        +[&] {
            return m_outfitService.DeleteConflictArmors(editingOutfit.GetName(), armor);
        };
    }
    +[&] {
        return m_outfitService.AddArmor(editingOutfit.GetId(), editingOutfit.GetName(), armor);
    };
}

auto OutfitEditPanel::IsArmorCanDisplay(const Armor *armor) -> bool
{
    bool canDisplay = false;

    if (armor != nullptr && armor->templateArmor == nullptr)
    {
        if (const std::string_view name = armor->GetName(); !name.empty())
        {
            canDisplay = true;
        }
    }

    return canDisplay;
}

void OutfitEditPanel::OnAcceptAddArmorToOutfit(const EditingOutfit &editingOutfit, const Armor *armor)
{
    if (editingOutfit.IsConflictWith(armor))
    {
        m_ConflictArmorPopup.conflictedArmor = armor;
        m_ConflictArmorPopup.Open();
    }
    else
    {
        +[&] {
            return m_outfitService.AddArmor(editingOutfit.GetId(), editingOutfit.GetName(), armor);
        };
        m_armorView.remove_armor(armor);
        m_armorView.multiSelection.Clear();
    }
}

void OutfitEditPanel::RenderPopups(const EditingOutfit &editingOutfit)
{
    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
    ImGui::PushFontSize(Config::FONT_SIZE_TITLE_4);
    bool confirmed = false;
    if (m_ConflictArmorPopup.Draw(confirmed); confirmed)
    {
        +[&] {
            return m_outfitService.AddArmor(editingOutfit.GetId(), editingOutfit.GetName(),
                                            m_ConflictArmorPopup.conflictedArmor);
        };
        m_armorView.remove_armor(m_ConflictArmorPopup.conflictedArmor);
        m_armorView.multiSelection.Clear();
        m_ConflictArmorPopup.conflictedArmor = nullptr;
    }

    if (m_DeleteArmorPopup.Draw(confirmed); confirmed)
    {
        +[&] {
            return m_outfitService.DeleteArmor(editingOutfit.GetId(), editingOutfit.GetName(),
                                               m_DeleteArmorPopup.wantDeleteArmor);
        };
        assert(m_armorView.add_armor(m_DeleteArmorPopup.wantDeleteArmor).has_value() && "Can't add armor!");
        m_DeleteArmorPopup.wantDeleteArmor = nullptr;
    }
    m_slotPolicyHelp.Draw();
    confirmed = false;
    if (m_batchAddArmorsPopUp.Draw(confirmed); confirmed)
    {
        BatchAddArmors(editingOutfit);
    }

    ImGui::PopFontSize();
    ImGui::PopStyleVar();
}

}