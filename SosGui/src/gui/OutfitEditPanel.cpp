#include "gui/OutfitEditPanel.h"
#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "data/ArmorGenerator.h"
#include "data/ArmorView.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/Popup.h"
#include "gui/Table.h"
#include "gui/widgets.h"
#include "imgui.h"
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
#include <list>
#include <mutex>
#include <string>
#include <ranges>
#include <unordered_set>

namespace
LIBC_NAMESPACE_DECL
{
constexpr auto SELECTABLE_SPAN_FLAGS = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;

void OutfitEditPanel::EditContext::Clear()
{
    armorView.Clear();
    armorViewData.clear();
    slotArmorCounter.fill(0);
    selectedArmor = nullptr;
    armorAddPolicy = 0;
    selectedFilterSlot = Slot::kNone;
    checkSlotAll = true;
    prevCheckAllSlot = true;
    showOutfitWindow = false;
    armorMultiSelection.Clear();
    candidateSelectedSlot = Slot::kNone;
    armorFilter.clear();
    dirty = true;
}

auto OutfitEditPanel::Render(const SosUiData::OutfitPair &wantEdit) -> bool
{
    if (m_editContext.dirty)
    {
        m_editContext.dirty = false;
        view_add_armors_by_policy();
    }
    ImGui::PushID(this);
    RenderPopups(wantEdit);
    RenderProperties(wantEdit);
    RenderArmorList(wantEdit);
    RenderEditPanel(wantEdit);
    ImGui::PopID();
    return !m_editContext.showOutfitWindow;
}

void OutfitEditPanel::Refresh() {}

void OutfitEditPanel::Close()
{
    m_editContext.Clear();
}

void OutfitEditPanel::OnSelectActor(const RE::Actor *actor, const SosUiOutfit *editingOutfit)
{
    if (editingOutfit == nullptr)
    {
        return;
    }
    switch (m_editContext.armorAddPolicy)
    {
        case OutfitAddPolicy_AddFromCarried:
        case OutfitAddPolicy_AddFromInventory: {
            view_add_armors_by_policy();
            view_remove_armors_in_outfit(editingOutfit);
            break;
        }
        default:
            break;
    }
}

void OutfitEditPanel::OnSelectOutfit(const SosUiOutfit *lastEditOutfit, const SosUiOutfit *editingOutfit)
{
    ShowWindow(editingOutfit->GetName());
    if (lastEditOutfit == nullptr)
    {
        view_init();
        view_add_armors_by_policy();
        view_remove_armors_in_outfit(editingOutfit);
    }
    else
    {
        switch (m_editContext.armorAddPolicy)
        {
            case OutfitAddPolicy_AddFromCarried:
            case OutfitAddPolicy_AddFromInventory: {
                view_add_armors_by_policy();
                view_remove_armors_in_outfit(editingOutfit);
                break;
            }
            case OutfitAddPolicy_AddAny:
                if (lastEditOutfit->GetId() != editingOutfit->GetId())
                {
                    view_add_armors_in_outfit(lastEditOutfit);
                    view_remove_armors_in_outfit(editingOutfit);
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

void OutfitEditPanel::RenderProperties(const SosUiData::OutfitPair &wantEdit) const
{
    static std::array<char, 256> outfitNameBuf;

    ImGuiUtil::InputText("##OutfitRenameInput", outfitNameBuf);
    ImGui::SameLine();
    ImGui::BeginDisabled(outfitNameBuf.at(0) == '\0');
    if (ImGuiUtil::Button("$SkyOutSys_OContext_Rename"))
    {
        +[&] {
            return m_outfitService.RenameOutfit(wantEdit.first, wantEdit.second->GetName(), outfitNameBuf.data());
        };
    }
    ImGui::EndDisabled();
}

void OutfitEditPanel::RenderArmorList(const SosUiData::OutfitPair &wantEdit)
{
    if (wantEdit.second->IsEmpty())
    {
        ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
        ImGuiUtil::Text("$SosGui_EmptyHint{$ARMOR}");
        ImGui::PopFontSize();
        return;
    }

    static bool showAllSlots = false;
    ImGuiUtil::CheckBox("$SosGui_CheckBox_SlotFilter", &showAllSlots);
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
            if (!showAllSlots && armor == nullptr)
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
                HighlightConflictArmor(armor);
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
                    m_editContext.selectedArmor = armor;
                    m_DeleteArmorPopup.Open();
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndTable();
    }
}

void OutfitEditPanel::HighlightConflictArmor(const Armor *armor) const
{
    if (m_editContext.candidateSelectedSlot.any(armor->GetSlotMask()))
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

void OutfitEditPanel::RenderEditPanel(const SosUiData::OutfitPair &wantEdit)
{
    RenderArmorSlotFilter(wantEdit);

    {
        const ImGuiStyle &style = ImGui::GetStyle();
        float halfWidth = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2;
        constexpr auto flags = ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders;
        ImGuiUtil::ChildGuard child("##RenderArmorCandidates", {halfWidth, 0.0F}, flags);
        RenderArmorCandidates(wantEdit);
    }
    const auto lastItemSize = ImGui::GetItemRectSize();
    if (const auto contentSize = ImGui::GetContentRegionAvail(); lastItemSize.x < contentSize.x)
    {
        ImGui::SameLine();
        ImGui::BeginGroup();
        RenderEditPanelPolicy(wantEdit);
        ImGui::EndGroup();
    }
}

static std::array<std::string, OutfitEditPanel::SLOT_COUNT> slotNames;
static std::once_flag init_slot_name_flag;

void OutfitEditPanel::init_slot_name()
{
    for (uint32_t idx = 0; idx < SLOT_COUNT; ++idx)
    {
        auto nameKey = std::format("$SkyOutSys_BodySlot{}", idx + SOS_SLOT_OFFSET);
        slotNames.at(idx) = Translation::Translate(nameKey);
    }
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
    std::string armorName = armor->GetName();
    std::string modName;
    if (const auto *modFile = armor->GetFile(); modFile != nullptr)
    {
        modName.assign(modFile->GetFilename());
    }
    if (debounce_input::PassFilter(armorName.c_str()))
    {
        return true;
    }
    if (debounce_input::PassFilter(modName.c_str()))
    {
        return true;
    }
    return false;
}

bool OutfitEditPanel::ArmorFilter::draw()
{
    bool checked = ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &mustPlayable);
    ImGui::SameLine();
    return checked || debounce_input::draw();
}

void OutfitEditPanel::RenderArmorSlotFilter(const SosUiData::OutfitPair &wantEdit)
{
    static std::unordered_set<uint32_t> selectedSlotPos;

    std::call_once(init_slot_name_flag, init_slot_name);

    {
        auto newEquipIndex = RE::BIPED_OBJECT::kNone;
        if (ImGuiUtil::BeginCombo("$SosGui_Combo_SlotFilter", nullptr))
        {
            for (uint8_t idx = 0; idx < RE::BIPED_OBJECT::kEditorTotal; ++idx)
            {
                ImGuiUtil::PushIdGuard idGuard(idx);
                if (ImGui::Selectable(slotNames[idx].c_str(), false))
                {
                    selectedSlotPos.insert(idx);
                    m_editContext.selectedFilterSlot.set(static_cast<Slot>(1 << idx));
                    newEquipIndex = static_cast<RE::BIPED_OBJECT>(idx);
                }
            }
            ImGui::EndCombo();
        }
        if (newEquipIndex < RE::BIPED_OBJECT::kEditorTotal)
        {
            if (m_editContext.checkSlotAll)
            {
                m_editContext.armorViewData.clear();
                m_editContext.checkSlotAll = false;
            }
            view_add_armors_has_slot(newEquipIndex);
        }
    }

    const auto contentWidth = ImGui::GetContentRegionAvail().x;
    auto cursorPosX = 0.0F;
    // const auto armorCount = RE::TESDataHandler::GetSingleton()->GetFormArray<Armor>().size();
    const auto armorCount = m_availableArmorCount;
    std::string name = std::format("All({}/{})##AllSlot", m_editContext.armorViewData.size(), armorCount);
    if (ImGui::Checkbox(name.c_str(), &m_editContext.checkSlotAll) && m_editContext.checkSlotAll)
    {
        m_editContext.selectedFilterSlot.set(static_cast<Slot>(UINT32_MAX));
        view_add_armors_by_policy();
    }
    const auto &style = ImGui::GetStyle();
    cursorPosX += ImGui::GetItemRectSize().x + style.ItemSpacing.x;
    ImGui::SameLine();

    auto beRemoveEquipIndex = RE::BIPED_OBJECT::kNone;
    for (auto itBegin = selectedSlotPos.begin(); itBegin != selectedSlotPos.end();)
    {
        const auto slot = static_cast<Slot>(1 << *itBegin);
        bool alwaysChecked = true;
        name = std::format("{}({})", slotNames[*itBegin], m_editContext.slotArmorCounter[*itBegin]);
        if (ImGui::Checkbox(name.c_str(), &alwaysChecked))
        {
            beRemoveEquipIndex = static_cast<RE::BIPED_OBJECT>(*itBegin);
            itBegin = selectedSlotPos.erase(itBegin);
            m_editContext.selectedFilterSlot.reset(slot);
        }
        else
        {
            ++itBegin;
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
    if (!m_editContext.checkSlotAll && beRemoveEquipIndex != RE::BIPED_OBJECT::kNone)
    {
        view_remove_armors_has_slot(m_editContext.selectedFilterSlot.get(), beRemoveEquipIndex);
    }
    ImGui::NewLine();
}

void OutfitEditPanel::RenderArmorCandidates(const SosUiData::OutfitPair &wantEdit)
{
    // filter armor name and mod name
    if (m_editContext.armorFilter.draw())
    {
        m_editContext.armorFilter.dirty = false;
        view_filter_reset(wantEdit.second);
    }

    if (constexpr auto flags = TableFlags().Resizable().SizingFixedFit().Sortable().Hideable().Reorderable().flags;
        m_editContext.armorViewData.empty() || !ImGui::BeginTable("##ArmorCandidates", 5, flags))
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

    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);

    static bool requireAdd = false;
    static bool requireAddAll = false;

    requireAddAll = false;
    auto drawAction = [&](Armor *armor, size_t index) {
        ImGuiUtil::PushIdGuard idGuard(index);
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) // number column
        {
            ImGui::Text("%.4zu", index + 1);
        }

        ImGui::TableNextColumn(); // armor name column
        {
            bool isSelected = m_editContext.armorMultiSelection.Contains(static_cast<ImGuiID>(index));
            ImGui::SetNextItemSelectionUserData(index);
            ImGui::Selectable(armor->GetName(), isSelected, SELECTABLE_SPAN_FLAGS);
            if (isSelected)
            {
                m_editContext.candidateSelectedSlot.set(armor->GetSlotMask());
            }
            else
            {
                m_editContext.candidateSelectedSlot.reset(armor->GetSlotMask());
            }
        }
        const bool prevRequireAddAll = requireAddAll;
        CandidateContextMenu(requireAddAll);
        if (!prevRequireAddAll && requireAddAll)
        {
            m_editContext.selectedArmor = armor;
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
            requireAdd = true;
            m_editContext.selectedArmor = armor;
        }
    };
    ImGuiListClipper clipper;
    clipper.Begin(m_editContext.armorViewData.size());
    // clang-format off
    auto *multiSelectionIo = m_editContext.armorMultiSelection
            .NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid()
            .Begin(clipper.ItemsCount);
    m_editContext.armorMultiSelection.ApplyRequests(multiSelectionIo);
    // clang-format on
    while (clipper.Step())
    {
        if (ascend)
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
            {
                drawAction(m_editContext.armorViewData.at(index), index);
            }
        }
        else
        {
            using namespace std::views;
            size_t index = static_cast<size_t>(clipper.DisplayStart);
            for (const auto &armor : m_editContext.armorViewData
                                     | reverse | drop(clipper.DisplayStart) | take(
                                         clipper.DisplayEnd - clipper.DisplayStart))
            {
                drawAction(armor, index++);
            }
        }
    }
    m_editContext.armorMultiSelection.ApplyRequests(ImGui::EndMultiSelect());
    ImGui::EndTable();

    // handle multi-selection
    if (requireAddAll && m_editContext.armorMultiSelection.Size > 0)
    {
        if (m_editContext.armorMultiSelection.Size == 1)
        {
            requireAdd = true; // back to add one select armor
        }
        else
        {
            m_batchAddArmorsPopUp.Open();
        }
    }

    if (requireAdd)
    {
        requireAdd = false;
        OnAddArmor(wantEdit, m_editContext.selectedArmor);
    }
}

void OutfitEditPanel::CandidateContextMenu(bool &acceptAddAll)
{
    if (!ImGui::BeginPopupContextItem())
    {
        return;
    }
    if (ImGuiUtil::MenuItem("$SosGui_ContextMenu_AddAllArmor"))
    {
        acceptAddAll = true;
    }
    ImGui::EndPopup();
}

void OutfitEditPanel::BatchAddArmors(const SosUiData::OutfitPair &wantEdit)
{
    SlotEnumeration usedSlot;
    void *it = nullptr;
    ImGuiID selectedRank; // must be name rank;
    while (m_editContext.armorMultiSelection.GetNextSelectedItem(&it, &selectedRank))
    {
        if (m_editContext.armorViewData.size() > selectedRank)
        {
            auto *armor = m_editContext.armorViewData.at(selectedRank);
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
    m_editContext.armorMultiSelection.Clear();
}

void OutfitEditPanel::RenderEditPanelPolicy(const SosUiData::OutfitPair &wantEdit)
{
    static std::array outfitPolicy{Translation::Translate("$SkyOutSys_OEdit_AddFromCarried"),
                                   Translation::Translate("$SkyOutSys_OEdit_AddFromWorn"),
                                   Translation::Translate("$SkyOutSys_OEdit_AddByID"),
                                   Translation::Translate("$SkyOutSys_OEdit_AddFromList_Header")};
    int idx = 0;
    bool fSelected = false;

    ImGui::NewLine();
    for (const auto &policy : outfitPolicy)
    {
        ImGuiUtil::PushIdGuard idGuard(idx);

        if (ImGui::RadioButton(policy.c_str(), idx == m_editContext.armorAddPolicy))
        {
            fSelected = m_editContext.armorAddPolicy != idx;
            m_editContext.armorAddPolicy = idx;
        }
        idx++;
    }

    const auto selectedPolicy = static_cast<OutfitAddPolicy>(m_editContext.armorAddPolicy);
    if (fSelected)
    {
        view_add_armors_by_policy();
        view_remove_armors_in_outfit(wantEdit.second);
    }

    if (selectedPolicy == OutfitAddPolicy_AddByID)
    {
        RenderOutfitAddPolicyById(wantEdit, m_editContext.armorFilter.mustPlayable);
    }
}

void OutfitEditPanel::RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit,
                                                const bool &fFilterPlayable) const
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

void OutfitEditPanel::view_add_armors_by_policy()
{
    m_editContext.checkSlotAll = true;
    m_editContext.selectedFilterSlot = Slot::kNone;
    ArmorGenerator *generator = nullptr;
    GetArmorGeneratorFromPolicy(&generator);
    if (generator)
    {
        m_editContext.armorViewData.clear();
        generator->for_each([&](Armor *armor) {
            if (m_editContext.armorFilter.PassFilter(armor))
            {
                m_editContext.armorViewData.emplace_back(armor);
                slotCounterAdd(armor);
            }
        });
    }
}

void OutfitEditPanel::GetArmorGeneratorFromPolicy(ArmorGenerator **generator) const
{
    switch (static_cast<OutfitAddPolicy>(m_editContext.armorAddPolicy))
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

void OutfitEditPanel::view_init()
{
    auto *dataHandler = RE::TESDataHandler::GetSingleton();
    const auto &armorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();

    m_availableArmorCount = 0;
    for (const auto &armor : armorArray)
    {
        if (IsArmorCanDisplay(armor))
        {
            m_editContext.armorView.Insert(armor);
            m_availableArmorCount++;
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

bool OutfitEditPanel::eraseArmor(const Armor *armor)
{
    bool result = std::erase_if(m_editContext.armorViewData, [&](const Armor *a_armor) {
        return a_armor->formID == armor->formID;
    }) > 0;
    if (result)
    {

        slotCounterRemove(armor);
    }
    return result;
}

void OutfitEditPanel::view_add_armors_has_slot(RE::BIPED_OBJECT equipIndex)
{
    ArmorGenerator *generator = nullptr;
    GetArmorGeneratorFromPolicy(&generator);
    if (generator != nullptr)
    {
        generator->for_each([&](Armor *armor) {
            if (util::IsArmorHasAnySlotOf(armor, ToSlot(equipIndex))
                && m_editContext.armorFilter.PassFilter(armor))
            {
                m_editContext.armorViewData.emplace_back(armor);
            }
        });
    }
}

auto OutfitEditPanel::view_add_armor(Armor *armor) -> std::expected<void, error>
{
    if (m_editContext.armorFilter.PassFilter(armor))
    {
        auto &viewData = m_editContext.armorViewData;
        const auto armorRank = m_editContext.armorView.GetRank(armor->formID);
        size_t startPos = 0;
        size_t endPos = viewData.size();
        while (endPos - startPos > 0)
        {
            const size_t middle = (endPos - startPos) / 2;
            const auto rank = m_editContext.armorView.GetRank(viewData.at(middle)->formID);
            if (rank >= m_editContext.armorView.Size())
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
        slotCounterAdd(armor);
    }
    return {};
}

void OutfitEditPanel::view_remove_armors_has_slot(const Slot selectedSlots, RE::BIPED_OBJECT equipIndex)
{
    auto &viewData = m_editContext.armorViewData;
    for (auto itBegin = viewData.begin(); itBegin != viewData.end();)
    {
        if (auto *armor = *itBegin;
            armor->HasPartOf(ToSlot(equipIndex)) && util::IsArmorHasNoneSlotOf(armor, selectedSlots))
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

void OutfitEditPanel::view_add_armors_in_outfit(const SosUiOutfit *editingOutfit)
{
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (auto *armor = editingOutfit->GetArmorAt(slotPos);
            armor != nullptr)
        {
            if (auto result = view_add_armor(armor); !result.has_value())
            {
                log_error("unexpected error: ", static_cast<uint8_t>(result.error()));
            }
        }
    }
}

void OutfitEditPanel::view_remove_armors_in_outfit(const SosUiOutfit *editingOutfit)
{
    // may multi armor use the same slot
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (const auto *armor = editingOutfit->GetArmorAt(slotPos);
            armor != nullptr && eraseArmor(armor))
        {
            m_editContext.slotArmorCounter[slotPos] -= 1;
        }
    }
}

void OutfitEditPanel::view_filter_remove() // TODO
{
    auto &viewData = m_editContext.armorViewData;

    for (auto itBegin = viewData.begin(); itBegin != viewData.end();)
    {
        if (m_editContext.armorFilter.PassFilter(*itBegin))
        {
            ++itBegin;
        }
        else
        {
            slotCounterRemove(*itBegin);
            itBegin = viewData.erase(itBegin);
        }
    }
}

void OutfitEditPanel::view_filter_reset(const SosUiOutfit *editingOutfit)
{
    m_editContext.slotArmorCounter.fill(0);
    view_add_armors_by_policy();
    view_remove_armors_in_outfit(editingOutfit);
}

inline void OutfitEditPanel::slotCounterAdd(const Armor *armor)
{
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (armor->HasPartOf(ToSlot(slotPos)))
        {
            m_editContext.slotArmorCounter[slotPos]++;
        }
    }
}

inline void OutfitEditPanel::slotCounterRemove(const Armor *armor)
{
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (armor->HasPartOf(ToSlot(slotPos)))
        {
            m_editContext.slotArmorCounter[slotPos]--;
        }
    }
}

void OutfitEditPanel::OnAddArmor(const SosUiData::OutfitPair &wantEdit, Armor *armor)
{
    if (wantEdit.second->IsConflictWith(armor))
    {
        m_ConflictArmorPopup.Open();
    }
    else
    {
        +[&] {
            return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
        };
        eraseArmor(armor);
    }
}

void OutfitEditPanel::RenderPopups(const SosUiData::OutfitPair &wantEdit)
{
    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
    ImGui::PushFontSize(HintFontSize());
    if (m_ConflictArmorPopup.Render(m_editContext.selectedArmor))
    {
        +[&] {
            return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), m_editContext.selectedArmor);
        };
        std::erase_if(m_editContext.armorViewData, [&](const Armor *a_armor) {
            return a_armor->formID == m_editContext.selectedArmor->formID;
        });
    }

    if (m_DeleteArmorPopup.Render(m_editContext.selectedArmor))
    {
        +[&] {
            return m_outfitService.DeleteArmor(wantEdit.first, wantEdit.second->GetName(), m_editContext.selectedArmor);
        };
        assert(view_add_armor(m_editContext.selectedArmor).has_value() && "Can't add armor!");
    }
    m_slotPolicyHelp.Render();
    if (m_batchAddArmorsPopUp.Render())
    {
        BatchAddArmors(wantEdit);
    }

    ImGui::PopFontSize();
    ImGui::PopStyleVar();
    if (m_ConflictArmorPopup.IsLastClosed() || m_DeleteArmorPopup.IsLastClosed())
    {
        m_editContext.selectedArmor = nullptr;
    }
}
}