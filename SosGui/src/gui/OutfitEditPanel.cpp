#include "gui/OutfitEditPanel.h"
#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "data/ArmorContainer.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/Popup.h"
#include "gui/Table.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "util/ImGuiUtil.h"
#include "util/PageUtil.h"
#include "util/utils.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <ranges>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
constexpr auto SELECTABLE_SPAN_FLAGS = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;

void OutfitEditPanel::EditContext::Clear()
{
    armorListShowAllSlotArmors = false;
    armorGeneratorSelectedActor = nullptr;
    dirty = true;
}

void OutfitEditPanel::Render(const SosUiData::OutfitPair &wantEdit)
{
    if (m_editContext.dirty)
    {
        m_editContext.dirty = false;
        AddArmorsToViewByGenerator(m_armorView, m_armorGeneratorTabBar.generator.get());
    }
    ImGui::PushID(this);
    RenderPopups(wantEdit);
    DrawOutfitArmors(wantEdit);
    m_armorGeneratorTabBar.Draw(m_armorView, m_uiData);
    DrawArmorViewSlotFilterer();
    DrawArmorView(wantEdit);
    ImGui::PopID();
}

void OutfitEditPanel::Refresh() {}

void OutfitEditPanel::Close()
{
    m_editContext.Clear();
    m_armorView.clear();
}

void OutfitEditPanel::OnSelectActor(const RE::Actor *, const SosUiOutfit *editingOutfit)
{
    if (editingOutfit == nullptr)
    {
        return;
    }
    switch (m_armorView.addPolicy)
    {
        case OutfitAddPolicy_AddFromCarried:
        case OutfitAddPolicy_AddFromInventory: {
            AddArmorsToViewByGenerator(m_armorView, m_armorGeneratorTabBar.generator.get());
            m_armorView.remove_armors_in_outfit(editingOutfit);
            break;
        }
        default:
            break;
    }
}

void OutfitEditPanel::OnSelectOutfit(const SosUiOutfit *lastEditOutfit, const SosUiOutfit *editingOutfit)
{
    UpdateWindowTitle(editingOutfit->GetName());
    if (lastEditOutfit == nullptr)
    {
        m_armorView.init();
        AddArmorsToViewByGenerator(m_armorView, m_armorGeneratorTabBar.generator.get());
        m_armorView.remove_armors_in_outfit(editingOutfit);
    }
    else
    {
        switch (m_armorView.addPolicy)
        {
            case OutfitAddPolicy_AddFromCarried:
            case OutfitAddPolicy_AddFromInventory: {
                AddArmorsToViewByGenerator(m_armorView, m_armorGeneratorTabBar.generator.get());
                m_armorView.remove_armors_in_outfit(editingOutfit);
                break;
            }
            case OutfitAddPolicy_AddAny:
                if (lastEditOutfit->GetId() != editingOutfit->GetId())
                {
                    m_armorView.add_armors_in_outfit(lastEditOutfit);
                    m_armorView.remove_armors_in_outfit(editingOutfit);
                }
                break;
            default:
                break;
        }
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

void OutfitEditPanel::DrawOutfitArmors(const SosUiData::OutfitPair &wantEdit)
{
    if (wantEdit.second->IsEmpty())
    {
        ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
        ImGuiUtil::Text("$SosGui_EmptyHint{$ARMOR}");
        ImGui::PopFontSize();
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

        for (uint32_t slotIdx = 0; slotIdx < SLOT_COUNT; ++slotIdx)
        {
            const auto &armor = wantEdit.second->GetArmorAt(slotIdx);
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
                auto name = std::format("$SkyOutSys_BodySlot{}", slotIdx + SOS_SLOT_OFFSET);
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
            SlotPolicyCombo(wantEdit, slotIdx);

            ImGui::TableNextColumn(); // action column
            ImGui::BeginDisabled(armor == nullptr);
            {
                if (ImGuiUtil::Button("$Delete"))
                {
                    m_DeleteArmorPopup.wantDeleteArmor = armor;
                    m_DeleteArmorPopup.Open();
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndTable();
    }
}

void OutfitEditPanel::HighlightConflictSlot(const Slot slot) const
{
    if (m_armorView.multiSelectedSlot.any(slot))
    {
        auto *drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 0, 0, 255), 0,
                          ImDrawFlags_None, 2.0F);
    }
}

void OutfitEditPanel::SlotPolicyCombo(const SosUiData::OutfitPair &wantEdit, const uint32_t &slotIdx) const
{
    auto &policyNameKey = wantEdit.second->GetSlotPolicies().at(slotIdx);
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
                SlotPolicyToCode(policy);
                +[&] {
                    return m_outfitService.SetSlotPolicy(wantEdit.first, wantEdit.second->GetName(), slotIdx, policy);
                };
            }
            ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
        }
        ImGui::EndCombo();
    }
}

auto OutfitEditPanel::get_slot_name_key(const uint32_t slotPos) -> std::string
{
    return std::format("$SkyOutSys_BodySlot{}", slotPos + SOS_SLOT_OFFSET);
}

bool OutfitEditPanel::ArmorFilter::PassFilter(const Armor *armor) const
{
    if (mustPlayable && IsArmorNonPlayable(armor))
    {
        return false;
    }
    if (!filter.IsActive())
    {
        return true;
    }
    if (DebounceInput::PassFilter(armor->GetName()))
    {
        return true;
    }
    if (const auto *modFile = armor->GetFile(); modFile != nullptr)
    {
        if (DebounceInput::PassFilter(modFile->GetFilename().data()))
        {
            return true;
        }
    }
    return false;
}

bool OutfitEditPanel::ArmorFilter::Draw()
{
    bool needUpdate = ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &mustPlayable);
    ImGui::SameLine();
    needUpdate |= DebounceInput::Draw("##ArmorFilter",
                                      Translation::Translate("$SkyOutSys_OEdit_AddFromList_Filter_Name").c_str());
    return needUpdate;
}

void OutfitEditPanel::ArmorView::clear()
{
    armorContainer.Clear();
    viewData.clear();
    slotCounter.fill(0);
    selectedFilterSlot = 0;
    checkAllSlot = true;
    multiSelection.Clear();
    multiSelectedSlot = Slot::kNone;
    armorFilter.clear();
    addPolicy = 0;
}

void OutfitEditPanel::ArmorView::clearViewData()
{
    viewData.clear();
    slotCounter.fill(0);
}

void OutfitEditPanel::DrawArmorViewTableContent(const std::function<void(Armor *armor, size_t index)> &drawAction)
{
    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);
    ImGuiListClipper clipper;
    // clang-format off
    auto *msIO = m_armorView.multiSelection
                            .NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid()
                            .Begin(m_armorView.viewData.size());
    clipper.Begin(m_armorView.viewData.size());
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
                drawAction(m_armorView.viewData.at(index), index);
            }
        }
        else
        {
            using namespace std::views;
            size_t index = static_cast<size_t>(clipper.DisplayStart);
            for (int count = clipper.DisplayEnd - clipper.DisplayStart;
                 const auto &armor : m_armorView.viewData | reverse | drop(clipper.DisplayStart) | take(count))
            {
                drawAction(armor, index++);
            }
        }
    }
    m_armorView.multiSelection.ApplyRequests(ImGui::EndMultiSelect());
}

void OutfitEditPanel::DrawArmorView(const SosUiData::OutfitPair &wantEdit)
{
    // filter armor name and mod name
    if (m_armorView.armorFilter.Draw())
    {
        m_armorView.armorFilter.dirty = false;
        AddArmorsToViewByGenerator(m_armorView, m_armorGeneratorTabBar.generator.get());
        m_armorView.remove_armors_in_outfit(wantEdit.second);
    }

    if (constexpr auto flags = TableFlags().Resizable().SizingFixedFit().Sortable().Hideable().Reorderable().flags;
        m_armorView.viewData.empty() || !ImGui::BeginTable("##ArmorCandidates", 6, flags))
    {
        return;
    }

    // clang-format off
    TableHeadersBuilder().Column("##Number").NoSort().WidthFixed()
        .Column("$ARMOR").DefaultSort().NoHide()
        .Column("FormID").WidthFixed().NoSort()
        .Column("ModName").NoSort()
        .Column("Playable").NoSort()
        .Column("$Add").WidthFixed().NoSort()
        .CommitHeadersRow();
    // clang-format on

    std::function<void()> onRequireAddArmor = [] {};

    auto drawAction = [&](Armor *armor, const size_t index) {
        ImGuiUtil::PushIdGuard idGuard(index);
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) // number column
        {
            ImGui::Text("%.4zu", index + 1);
        }

        ImGui::TableNextColumn(); // armor name column
        {
            const bool isSelected = m_armorView.multiSelection.Contains(static_cast<ImGuiID>(index));
            ImGui::SetNextItemSelectionUserData(index);
            ImGui::Selectable(armor->GetName(), isSelected, SELECTABLE_SPAN_FLAGS);
            if (isSelected)
            {
                m_armorView.multiSelectedSlot.set(armor->GetSlotMask());
            }
            else
            {
                m_armorView.multiSelectedSlot.reset(armor->GetSlotMask());
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGuiUtil::MenuItem("$SosGui_ContextMenu_AddAllArmor"))
                {
                    if (m_armorView.multiSelection.Size == 1)
                    {
                        onRequireAddArmor = [&, armor] {
                            OnAddArmor(wantEdit, armor);
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

        if (ImGui::TableNextColumn()) // formId column
        {
            ImGui::Text("%s", std::format("{:#010X}", armor->GetFormID()).c_str());
        }

        if (ImGui::TableNextColumn()) // mod name column
        {
            ImGui::Text("%s", util::GetArmorModFileName(armor).data());
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                ImGui::SetItemTooltip("%s", util::GetArmorModFileName(armor).data());
            }
        }

        if (ImGui::TableNextColumn()) // column playable
        {
            ImGui::Text("%s", IsArmorNonPlayable(armor) ? "\xe2\x9d\x8c" : "\xe2\x9c\x85");
        }

        if (ImGui::TableNextColumn() && ImGuiUtil::Button("$Add")) // column Action
        {
            onRequireAddArmor = [&, armor] {
                OnAddArmor(wantEdit, armor);
            };
        }
    };

    DrawArmorViewTableContent(drawAction);
    ImGui::EndTable();
    onRequireAddArmor();
}

void OutfitEditPanel::DrawArmorViewSlotFilterer()
{
    if (ImGui::BeginCombo("##SlotFilter", Translation::Translate("$SosGui_Combo_SlotFilter").c_str(),
                          ImGuiComboFlags_WidthFitPreview))
    {
        for (uint8_t idx = 0; idx < RE::BIPED_OBJECT::kEditorTotal; ++idx)
        {
            ImGuiUtil::PushIdGuard idGuard(idx);
            if (ImGuiUtil::Selectable(get_slot_name_key(idx).c_str(), false))
            {
                m_armorView.selectedFilterSlot.set(idx);
                if (m_armorView.checkAllSlot)
                {
                    m_armorView.clearViewData();
                    m_armorView.checkAllSlot = false;
                }
                m_armorView.add_armors_has_slot(ToSlot(idx));
            }
        }
        ImGui::EndCombo();
    }

    const auto contentWidth = ImGui::GetContentRegionAvail().x;
    auto cursorPosX = 0.0F;
    std::string name = std::format("All({}/{})##AllSlot", m_armorView.viewData.size(), m_armorView.availableArmorCount);
    if (ImGui::Checkbox(name.c_str(), &m_armorView.checkAllSlot) && m_armorView.checkAllSlot)
    {
        AddArmorsToViewByGenerator(m_armorView, m_armorGeneratorTabBar.generator.get());
    }
    const auto &style = ImGui::GetStyle();
    cursorPosX += ImGui::GetItemRectSize().x + style.ItemSpacing.x;
    ImGui::SameLine();

    for (size_t idx = 0; idx < m_armorView.selectedFilterSlot.size(); ++idx)
    {
        if (!m_armorView.selectedFilterSlot.test(idx)) continue;

        bool alwaysChecked = true;
        name = std::format("{}({})", Translation::Translate(get_slot_name_key(idx)), m_armorView.slotCounter[idx]);
        if (ImGui::Checkbox(name.c_str(), &alwaysChecked))
        {
            m_armorView.selectedFilterSlot.reset(idx);
            if (!m_armorView.checkAllSlot && m_armorView.selectedFilterSlot == 0)
            {
                m_armorView.checkAllSlot = true;
                AddArmorsToViewByGenerator(m_armorView, m_armorGeneratorTabBar.generator.get());
            }
            else
            {
                m_armorView.remove_armors_has_slot(ToSlot(m_armorView.selectedFilterSlot.to_ulong()), ToSlot(idx));
            }
        }
        const auto itemX = ImGui::GetItemRectSize().x;
        if (const auto nextPos = cursorPosX + itemX + style.ItemSpacing.x * 2.0F; nextPos <= contentWidth)
        {
            ImGui::SameLine();
            cursorPosX = nextPos;
        }
        else
        {
            cursorPosX = 0.0F;
        }
    }
    ImGui::NewLine();
}

void OutfitEditPanel::AddArmorsToViewByGenerator(ArmorView &view, ArmorGenerator *generator)
{
    view.clearViewData();
    generator->for_each([&](Armor *armor) {
        if (view.armorFilter.PassFilter(armor))
        {
            view.viewData.emplace_back(armor);
            view.slot_counter_add(armor);
        }
    });
}

void OutfitEditPanel::BatchAddArmors(const SosUiData::OutfitPair &wantEdit)
{
    SlotEnumeration usedSlot;
    void *it = nullptr;
    ImGuiID selectedRank; // must be name rank;
    while (m_armorView.multiSelection.GetNextSelectedItem(&it, &selectedRank))
    {
        if (m_armorView.viewData.size() > selectedRank)
        {
            auto *armor = m_armorView.viewData.at(selectedRank);
            if (usedSlot.all(armor->GetSlotMask()))
            {
                continue;
            }
            usedSlot.set(armor->GetSlotMask());
            if (wantEdit.second->IsConflictWith(armor))
            {
                +[&] {
                    return m_outfitService.DeleteConflictArmors(wantEdit.second->GetName(), armor);
                };
            }
            +[&] {
                return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
            };
        }
    }
    m_armorView.multiSelection.Clear();
}

auto OutfitEditPanel::RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit,
                                                const bool &fFilterPlayable) const -> void
{
    static std::array<char, 32> formIdBuf;
    static char *pEnd{};

    ImGui::PushID("AddByFormId");
    ImGui::Text("0x");
    ImGui::SameLine();
    ImGui::InputText("##InputArmorId", formIdBuf.data(), formIdBuf.size(),
                     ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal);
    const auto formId = std::strtoul(formIdBuf.data(), &pEnd, 16);
    if (Armor *armor = nullptr; *pEnd == 0 && (armor = RE::TESForm::LookupByID<Armor>(formId)) != nullptr)
    {
        ImGui::Text("%s", armor->GetName());
        ImGui::SameLine();
        ImGui::BeginDisabled(fFilterPlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0);
        if (ImGuiUtil::Button("$Add"))
        {
            +[&] {
                return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
            };
        }
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    ImGui::PopID();
}

// void OutfitEditPanel::ArmorView::add_armors_by_policy()
// {
//     checkAllSlot = true;
//     selectedFilterSlot = 0;
//     ArmorGenerator *generator = nullptr;
//     GetArmorGeneratorFromPolicy(static_cast<OutfitAddPolicy>(addPolicy), &generator);
//     if (generator)
//     {
//         viewData.clear();
//         generator->for_each([&](Armor *armor) {
//             if (armorFilter.PassFilter(armor))
//             {
//                 viewData.emplace_back(armor);
//                 slot_counter_add(armor);
//             }
//         });
//     }
// }

void OutfitEditPanel::GetArmorGeneratorFromPolicy(const OutfitAddPolicy policy, ArmorGenerator **generator)
{
    switch (policy)
    {
        case OutfitAddPolicy_AddFromCarried:
            *generator = new CarriedArmorGenerator(RE::PlayerCharacter::GetSingleton());
            break;

        case OutfitAddPolicy_AddFromInventory:
            *generator = new InventoryArmorGenerator(RE::PlayerCharacter::GetSingleton());
            break;

        case OutfitAddPolicy_AddByID:
            break;

        case OutfitAddPolicy_AddAny:
            *generator = new BasicArmorGenerator();
            break;

        default:
            break;
    }
}

void OutfitEditPanel::ArmorView::init()
{
    auto *dataHandler = RE::TESDataHandler::GetSingleton();
    const auto &armorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();

    availableArmorCount = 0;
    for (const auto &armor : armorArray)
    {
        if (IsArmorCanDisplay(armor))
        {
            armorContainer.Insert(armor);
            availableArmorCount++;
        }
    }
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

bool OutfitEditPanel::ArmorView::eraseArmor(const Armor *armor)
{
    const bool result = std::erase_if(viewData, [&](const Armor *a_armor) {
        return a_armor->formID == armor->formID;
    }) > 0;
    if (result)
    {
        slot_counter_remove(armor);
    }
    return result;
}

void OutfitEditPanel::ArmorGeneratorTabBar::Draw(ArmorView &armorView, const SosUiData &uiData)
{
    using namespace ImGuiUtil;
    if (ImGui::BeginTabBar("ArmorGeneratorTabBar"), TabBarFlags().DrawSelectedOverline().Reorderable().flags)
    {
        const auto *tabBar = ImGui::GetCurrentTabBar();
        const auto nextTabId = tabBar->NextSelectedTabId;
        const auto selectedId = tabBar->SelectedTabId;
        auto isTabItemAppear = [&] {
            const ImGuiTabItem *tabItem = ImGui::TabBarGetCurrentTab(ImGui::GetCurrentTabBar());
            return selectedId != nextTabId && tabItem->ID == nextTabId;
        };
        auto *player = RE::PlayerCharacter::GetSingleton();
        if (ImGui::BeginTabItem("From Actor Inventory"))
        {
            if (isTabItemAppear())
            {
                generator = std::make_unique<InventoryArmorGenerator>(selectedActor);
                AddArmorsToViewByGenerator(armorView, generator.get());
            }
            Text("$SosGui_Actor_ArmorSource");
            ImGui::SameLine();
            if (widgets::DrawNearActorsCombo(uiData.GetNearActors(), &selectedActor, player))
            {
                generator = std::make_unique<InventoryArmorGenerator>(selectedActor);
                AddArmorsToViewByGenerator(armorView, generator.get());
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("From Actor Carried Items"))
        {
            if (isTabItemAppear())
            {
                generator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                AddArmorsToViewByGenerator(armorView, generator.get());
            }
            Text("$SosGui_Actor_ArmorSource");
            ImGui::SameLine();
            if (widgets::DrawNearActorsCombo(uiData.GetNearActors(), &selectedActor, player))
            {
                generator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                AddArmorsToViewByGenerator(armorView, generator.get());
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("From form-id"))
        {
            // TODO
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("All Armors"))
        {
            if (isTabItemAppear())
            {
                generator = std::make_unique<BasicArmorGenerator>();
                AddArmorsToViewByGenerator(armorView, generator.get());
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::ArmorView::add_armors_has_slot(const Slot newSlot)
{
    ArmorGenerator *generator = nullptr;
    GetArmorGeneratorFromPolicy(static_cast<OutfitAddPolicy>(addPolicy), &generator);
    if (generator != nullptr)
    {
        generator->for_each([&](Armor *armor) {
            if (util::IsArmorHasAnySlotOf(armor, newSlot) && armorFilter.PassFilter(armor))
            {
                viewData.emplace_back(armor);
                slot_counter_add(armor);
            }
        });
    }
}

auto OutfitEditPanel::ArmorView::add_armor(Armor *armor) -> std::expected<void, error>
{
    if (armorFilter.PassFilter(armor))
    {
        const auto armorRank = armorContainer.GetRank(armor->formID);
        size_t startPos = 0;
        size_t endPos = viewData.size();
        while (endPos - startPos > 0)
        {
            const size_t middle = (endPos - startPos) / 2;
            const auto rank = armorContainer.GetRank(viewData.at(middle)->formID);
            if (rank >= armorContainer.Size())
            {
                return std::unexpected{error::unassociated_armor};
            }
            if (armorRank < rank)
            {
                endPos = middle;
            }
            else if (armorRank > rank)
            {
                startPos = middle + 1;
            }
            else
            {
                return std::unexpected{error::armor_already_exists};
            }
        }
        viewData.insert(viewData.begin() + startPos, armor);
        slot_counter_add(armor);
    }
    return {};
}

void OutfitEditPanel::ArmorView::remove_armors_has_slot(const Slot selectedSlots, const Slot toRemoveSlot)
{
    for (auto itBegin = viewData.begin(); itBegin != viewData.end();)
    {
        if (const auto *armor = *itBegin;
            armor->HasPartOf(toRemoveSlot) && util::IsArmorHasNoneSlotOf(armor, selectedSlots))
        {
            itBegin = viewData.erase(itBegin);
            eraseArmor(armor);
        }
        else
        {
            ++itBegin;
        }
    }
}

void OutfitEditPanel::ArmorView::add_armors_in_outfit(const SosUiOutfit *editingOutfit)
{
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (auto *armor = editingOutfit->GetArmorAt(slotPos); armor != nullptr)
        {
            if (auto result = add_armor(armor); !result.has_value())
            {
                log_error("unexpected error: ", static_cast<uint8_t>(result.error()));
            }
        }
    }
}

void OutfitEditPanel::ArmorView::remove_armors_in_outfit(const SosUiOutfit *editingOutfit)
{
    // may multi armor use the same slot
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (const auto *armor = editingOutfit->GetArmorAt(slotPos); armor != nullptr && eraseArmor(armor))
        {
            slotCounter[slotPos] -= 1;
        }
    }
}

inline void OutfitEditPanel::ArmorView::slot_counter_add(const Armor *armor)
{
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (armor->HasPartOf(ToSlot(slotPos)))
        {
            slotCounter[slotPos]++;
        }
    }
}

inline void OutfitEditPanel::ArmorView::slot_counter_remove(const Armor *armor)
{
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (armor->HasPartOf(ToSlot(slotPos)))
        {
            slotCounter[slotPos]--;
        }
    }
}

void OutfitEditPanel::OnAddArmor(const SosUiData::OutfitPair &wantEdit, Armor *armor)
{
    if (wantEdit.second->IsConflictWith(armor))
    {
        m_ConflictArmorPopup.conflictedArmor = armor;
        m_ConflictArmorPopup.Open();
    }
    else
    {
        +[&] {
            return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
        };
        m_armorView.eraseArmor(armor);
    }
}

void OutfitEditPanel::RenderPopups(const SosUiData::OutfitPair &wantEdit)
{
    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
    ImGui::PushFontSize(HintFontSize());
    bool confirmed = false;
    if (m_ConflictArmorPopup.Draw(confirmed); confirmed)
    {
        +[&] {
            return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(),
                                            m_ConflictArmorPopup.conflictedArmor);
        };
        std::erase_if(m_armorView.viewData, [&](const Armor *a_armor) {
            return a_armor->formID == m_ConflictArmorPopup.conflictedArmor->formID;
        });
        m_ConflictArmorPopup.conflictedArmor = nullptr;
    }

    if (m_DeleteArmorPopup.Draw(confirmed); confirmed)
    {
        +[&] {
            return m_outfitService.DeleteArmor(wantEdit.first, wantEdit.second->GetName(),
                                               m_DeleteArmorPopup.wantDeleteArmor);
        };
        assert(m_armorView.add_armor(m_DeleteArmorPopup.wantDeleteArmor).has_value() && "Can't add armor!");
        m_DeleteArmorPopup.wantDeleteArmor = nullptr;
    }
    m_slotPolicyHelp.Draw();
    confirmed = false;
    if (m_batchAddArmorsPopUp.Draw(confirmed); confirmed)
    {
        BatchAddArmors(wantEdit);
    }

    ImGui::PopFontSize();
    ImGui::PopStyleVar();
}
}