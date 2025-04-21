#include "gui/OutfitEditPanel.h"
#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "data/ArmorGenerator.h"
#include "data/ArmorView.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/Popup.h"
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
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <list>
#include <mutex>
#include <string>
#include <unordered_set>

namespace LIBC_NAMESPACE_DECL
{
    constexpr auto SELECTABLE_SPAN_FLAGS = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;

    auto OutfitEditPanel::Render(const SosUiData::OutfitPair &wantEdit) -> bool
    {
        constexpr auto flags = ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushID(this);
        if (ImGui::Begin(m_windowTitle.c_str(), &m_editContext.showOutfitWindow, flags))
        {
            RenderPopups(wantEdit);
            RenderProperties(wantEdit);
            RenderArmorList(wantEdit);
            RenderEditPanel(wantEdit);
        }
        ImGui::End();
        ImGui::PopID();
        return !m_editContext.showOutfitWindow;
    }

    void OutfitEditPanel::UpdateWindowTitle(const std::string &outfitName)
    {
        m_windowTitle = Translation::Translate("$SosGui_WindowName_EditingOutfit");
        if (const auto pos = m_windowTitle.find("{}"); pos != std::string::npos)
        {
            m_windowTitle.replace(pos, 2, outfitName);
        }
    }

    void OutfitEditPanel::RenderProperties(const SosUiData::OutfitPair &wantEdit)
    {
        static std::array<char, 256> outfitNameBuf;

        ImGuiUtil::InputText("##OutfitRenameInput", outfitNameBuf);
        ImGui::SameLine();
        ImGui::BeginDisabled(outfitNameBuf.at(0) == '\0');
        if (ImGuiUtil::Button("$SkyOutSys_OContext_Rename"))
        {
            *this << m_outfitService.RenameOutfit(wantEdit.first, wantEdit.second->GetName(), outfitNameBuf.data());
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
        if (m_armorListTable.Begin())
        {
            // clang-format off
            m_armorListTable.Column(0).NoHide().Setup()
                .Column(1).NoSort().Setup()
                .Column(2).NoSort().Setup()
                .Column(3).WidthFixed().NoSort().Setup()
                .Column(4).WidthFixed().NoSort().Setup();
            // clang-format on
            ImGui::TableHeadersRow();
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

                ImGui::TableNextColumn(); // slot name column
                const bool  isSelected = static_cast<uint32_t>(selectedIdx) == slotIdx;
                auto        name       = std::format("$SkyOutSys_BodySlot{}", slotIdx + SOS_SLOT_OFFSET);
                static auto flags      = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;
                if (ImGuiUtil::Selectable(name, isSelected, flags))
                {
                    selectedIdx = isSelected ? -1 : slotIdx;
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

    void OutfitEditPanel::HighlightConflictArmor(Armor *armor) const
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
        auto       &policyNameKey = wantEdit.second->GetSlotPolicies().at(slotIdx);
        std::string policyName;

        if (ImGui::BeginCombo("##SlotPolicy", Translation::Translate(policyNameKey).c_str()))
        {
            for (const auto &policy : {SlotPolicy::Inherit, SlotPolicy::Passthrough, SlotPolicy::RequireEquipped,
                                       SlotPolicy::AlwaysUseOutfit})
            {
                policyName = SlotPolicyToUiString(policy);
                if (ImGui::Selectable(policyName.c_str(), false))
                {
                    SlotPolicyToCode(policy);
                    *this << m_outfitService.SetSlotPolicy(wantEdit.first, wantEdit.second->GetName(), slotIdx, policy);
                }
                ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
            }
            ImGui::EndCombo();
        }
    }

    void OutfitEditPanel::RenderEditPanel(const SosUiData::OutfitPair &wantEdit)
    {
        RenderArmorSlotFilter(wantEdit);

        const ImGuiStyle &style     = ImGui::GetStyle();
        float             halfWidth = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2;
        {
            ImGuiUtil::ChildGuard child("##RenderArmorCandidates", {halfWidth, 0.0F});
            RenderArmorCandidates(wantEdit);
        }

        ImGui::SameLine();
        {
            ImGuiUtil::ChildGuard child("##RenderEditPanelPolicy", {halfWidth, 0.0F});
            RenderEditPanelPolicy(wantEdit);
        }
    }

    static std::array<std::string, OutfitEditPanel::SLOT_COUNT> slotNames;
    static std::once_flag                                       init_slot_name_flag;

    void OutfitEditPanel::init_slot_name()
    {
        for (uint32_t idx = 0; idx < SLOT_COUNT; ++idx)
        {
            auto nameKey      = std::format("$SkyOutSys_BodySlot{}", idx + SOS_SLOT_OFFSET);
            slotNames.at(idx) = Translation::Translate(nameKey);
        }
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
            if (newEquipIndex != RE::BIPED_OBJECT::kNone)
            {
                if (m_editContext.checkSlotAll)
                {
                    m_editContext.checkSlotAll = false;
                    m_editContext.armorView1.Clear();
                }
                view_add_armors_has_slot(newEquipIndex);
                view_filter_reset(wantEdit.second);
            }
        }

        const auto contentWidth = ImGui::GetContentRegionAvail().x;
        auto       cursorPosX   = 0.0F;
        // const auto  armorCount   = RE::TESDataHandler::GetSingleton()->GetFormArray<Armor>().size();
        const auto  armorCount = m_availableArmorCount;
        std::string name       = std::format("All({}/{})##AllSlot", m_editContext.armorView2.Size(), armorCount);
        if (ImGui::Checkbox(name.c_str(), &m_editContext.checkSlotAll) && m_editContext.checkSlotAll)
        {
            m_editContext.selectedFilterSlot.set(static_cast<Slot>(UINT32_MAX));
            view_add_armors_by_policy(wantEdit.second);
        }
        const auto &style = ImGui::GetStyle();
        cursorPosX += ImGui::GetItemRectSize().x + style.ItemSpacing.x;
        ImGui::SameLine();

        auto beRemoveEquipIndex = RE::BIPED_OBJECT::kNone;
        for (auto itBegin = selectedSlotPos.begin(); itBegin != selectedSlotPos.end();)
        {
            const auto slot          = static_cast<Slot>(1 << *itBegin);
            bool       alwaysChecked = true;
            name = std::format("{}({})", slotNames[*itBegin], m_editContext.slotArmorCounter[*itBegin]);
            if (ImGui::Checkbox(name.c_str(), &alwaysChecked))
            {
                beRemoveEquipIndex = static_cast<RE::BIPED_OBJECT>(*itBegin);
                itBegin            = selectedSlotPos.erase(itBegin);
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
        if (m_editContext.armorView2.IsEmpty())
        {
            ImGui::PushFontSize(HintFontSize());
            ImGuiUtil::Text(m_editContext.armorView2.IsEmpty() ? "$SosGui_EmptyHint{$ARMOR}" : "");
            ImGui::PopFontSize();
            return;
        }

        util::RenderPageWidgets(m_armorCandidatesPage);

        if (!m_armorCandidatesTable.Begin())
        {
            return;
        }

        ImGui::PushID("HeadersRow");
        // clang-format off
        m_armorCandidatesTable
            .Column(0).NoSort().WidthFixed().NoHide().Setup()
            .Column(1).DefaultSort().NoHide().Setup()
            .Column(2).WidthFixed().NoSort().Setup()
            .Column(3).NoSort().Setup()
            .Column(4).WidthFixed().NoSort().Setup();
        // clang-format on
        ImGui::TableHeadersRow();
        ImGui::PopID();

        if (auto *sortSpecs = ImGui::TableGetSortSpecs(); sortSpecs != nullptr)
        {
            if (sortSpecs->SpecsDirty)
            {
                assert(sortSpecs->SpecsCount == 1);
                const auto direction = sortSpecs->Specs[0].SortDirection;
                m_armorCandidatesPage.SetAscendSort(direction == ImGuiSortDirection_Ascending);
                sortSpecs->SpecsDirty = false;
            }
        }

        static bool requireAdd    = false;
        static bool requireAddAll = false;
        m_armorCandidatesPage.SetItemCount(m_editContext.armorView2.Size());

        // clang-format off
        auto *multiSelectionIo = m_editContext.candidateSelection
                .NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid()
                .Begin(m_armorCandidatesPage.GetPageSize());
        // clang-format on
        m_editContext.candidateSelection.ApplyRequests(multiSelectionIo);

        auto range     = m_armorCandidatesPage.PageRange();
        requireAddAll  = false;
        auto rowAction = [&](Armor *armor, size_t index) {
            ImGuiUtil::PushIdGuard idGuard(index);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%zu", index + 1);

            ImGui::TableNextColumn(); // armor name column

            bool isSelected = m_editContext.candidateSelection.Contains(static_cast<ImGuiID>(index));
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

            CandidateContextMenu(requireAddAll);
            if (requireAddAll)
            {
                m_editContext.selectedArmor = armor;
            }

            ImGui::TableNextColumn(); // formId column
            ImGui::Text("%s", std::format("{:#010X}", armor->GetFormID()).c_str());

            ImGui::TableNextColumn(); // mod name column
            ImGui::Text("%s", util::GetArmorModFileName(armor).data());

            ImGui::TableNextColumn(); // Action
            if (ImGuiUtil::Button("$Add"))
            {
                requireAdd                  = true;
                m_editContext.selectedArmor = armor;
            }
        };
        m_editContext.armorView2.for_each(m_armorCandidatesPage.IsAscend(), range.first, range.second, rowAction);
        ImGui::EndTable();
        multiSelectionIo = ImGui::EndMultiSelect();
        m_editContext.candidateSelection.ApplyRequests(multiSelectionIo);

        // handle multi-selection
        if (requireAddAll && m_editContext.candidateSelection.Size > 0)
        {
            if (m_editContext.candidateSelection.Size == 1)
            {
                requireAdd = true; // just back to add one select armor
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
        auto     range = m_armorCandidatesPage.PageRange();
        uint16_t index = 0;

        SlotEnumeration usedSlot;
        m_editContext.armorView2.for_each(range.first, range.second, [&](Armor *armor) {
            if (m_editContext.candidateSelection.Contains(index) && usedSlot.none(armor->GetSlotMask()))
            {
                usedSlot.set(armor->GetSlotMask());
                if (wantEdit.second->IsConflictWith(armor))
                {
                    *this << m_outfitService.DeleteConflictArmors(wantEdit.second->GetName(), armor);
                }
                *this << m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
            }
            ++index;
        });
        m_editContext.candidateSelection.Clear();
    }

    void OutfitEditPanel::RenderEditPanelPolicy(const SosUiData::OutfitPair &wantEdit)
    {
        static std::array outfitPolicy{Translation::Translate("$SkyOutSys_OEdit_AddFromCarried"),
                                       Translation::Translate("$SkyOutSys_OEdit_AddFromWorn"),
                                       Translation::Translate("$SkyOutSys_OEdit_AddByID"),
                                       Translation::Translate("$SkyOutSys_OEdit_AddFromList_Header")};
        int               idx       = 0;
        bool              fSelected = false;

        ImGui::NewLine();
        for (const auto &policy : outfitPolicy)
        {
            ImGuiUtil::PushIdGuard idGuard(idx);

            if (ImGui::RadioButton(policy.c_str(), idx == m_editContext.armorAddPolicy))
            {
                fSelected                    = m_editContext.armorAddPolicy != idx;
                m_editContext.armorAddPolicy = idx;
            }
            idx++;
        }

        const auto selectedPolicy         = static_cast<OutfitAddPolicy>(m_editContext.armorAddPolicy);
        bool       shouldUpdateCandidates = fSelected;
        if (fSelected)
        {
            if (selectedPolicy == OutfitAddPolicy_AddByID)
            {
                m_editContext.filterStringBuf[0] = '\0';
            }
        }

        if (ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &m_editContext.filterPlayable))
        {
            if (m_editContext.filterPlayable)
            {
                view_filter_remove();
            }
            else
            {
                view_filter_reset(wantEdit.second);
            }
        }
        if (selectedPolicy == OutfitAddPolicy_AddByID)
        {
            RenderOutfitAddPolicyById(wantEdit, m_editContext.filterPlayable);
            return;
        }

        // filter armor name and mod name
        static bool empty = false;
        ImGuiUtil::InputText("$SkyOutSys_OEdit_AddFromList_Filter_Name", m_editContext.filterStringBuf);
        if (ImGui::IsItemActivated() && ImGui::GetIO().WantTextInput)
        {
            empty = m_editContext.filterStringBuf[0] == '\0';
        }
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            if (empty)
            {
                view_filter_remove();
            }
            else
            {
                view_filter_reset(wantEdit.second);
            }
        }

        if (shouldUpdateCandidates)
        {
            view_add_armors_by_policy(wantEdit.second);
        }
    }

    void OutfitEditPanel::RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit, const bool &fFilterPlayable)
    {
        static std::array<char, 32> formIdBuf;
        static char                *pEnd{};

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
                *this << m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
            }
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }

    void OutfitEditPanel::view_add_armors_by_policy(const SosUiOutfit *outfit)
    {
        m_editContext.checkSlotAll       = true;
        m_editContext.selectedFilterSlot = Slot::kNone;
        ArmorGenerator *generator        = nullptr;
        GetArmorGeneratorFromPolicy(&generator);
        if (generator)
        {
            view_add_armors_with_generator(*generator);
            view_filter_reset(outfit);
        }
    }

    void OutfitEditPanel::GetArmorGeneratorFromPolicy(ArmorGenerator **generator) const
    {
        auto policy = static_cast<OutfitAddPolicy>(m_editContext.armorAddPolicy);
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

    auto OutfitEditPanel::IsArmorCanDisplay(Armor *armor) const -> bool
    {
        bool canDisplay = false;
        if (armor != nullptr && armor->templateArmor == nullptr)
        {
            if (std::string_view name = armor->GetName(); !name.empty())
            {
                canDisplay = true;
            }
        }

        return canDisplay;
    }

    void OutfitEditPanel::view_add_armors_has_slot(RE::BIPED_OBJECT equipIndex)
    {
        ArmorGenerator *generator = nullptr;
        GetArmorGeneratorFromPolicy(&generator);
        if (generator != nullptr)
        {
            generator->for_each([&](Armor *armor) {
                if (util::IsArmorHasAnySlotOf(armor, ToSlot(equipIndex)))
                {
                    m_editContext.armorView1.Insert(armor);
                }
            });
        }
    }

    void OutfitEditPanel::view_remove_armors_has_slot(Slot selectedSlots, RE::BIPED_OBJECT equipIndex)
    {
        for (auto itBegin = m_editContext.armorView1.begin(); itBegin != m_editContext.armorView1.end();)
        {
            auto *armor = *itBegin;
            if (armor->HasPartOf(ToSlot(equipIndex)) && util::IsArmorHasNoneSlotOf(armor, selectedSlots))
            {
                itBegin = m_editContext.armorView1.erase(itBegin);
                m_editContext.armorView2.Remove(armor);
            }
            else
            {
                ++itBegin;
            }
        }
    }

    void OutfitEditPanel::view_remove_armors_in_outfit(const SosUiOutfit *editingOutfit)
    {
        // may multi armor use the same slot
        for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            auto *armor = editingOutfit->GetArmorAt(slotPos);
            if (armor != nullptr && m_editContext.armorView2.Remove(armor))
            {
                m_editContext.slotArmorCounter[slotPos] -= 1;
            }
        }
    }

    void OutfitEditPanel::view_filter_remove()
    {
        auto &view2 = m_editContext.armorView2;

        for (auto itBegin = view2.begin(); itBegin != view2.end();)
        {
            if (IsFilterArmor(*itBegin))
            {
                for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; ++slotPos)
                {
                    if ((*itBegin)->HasPartOf(ToSlot(slotPos)))
                    {
                        m_editContext.slotArmorCounter[slotPos] -= 1;
                    }
                }
                itBegin = view2.erase(itBegin);
            }
            else
            {
                ++itBegin;
            }
        }
    }

    void OutfitEditPanel::view_filter_reset(const SosUiOutfit *editingOutfit)
    {
        auto &view2 = m_editContext.armorView2;
        view2.Clear();
        m_editContext.slotArmorCounter.fill(0);

        for (auto itBegin = m_editContext.armorView1.begin(); itBegin != m_editContext.armorView1.end(); ++itBegin)
        {
            if (IsFilterArmor(*itBegin))
            {
                continue;
            }
            else
            {
                for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; ++slotPos)
                {
                    if ((*itBegin)->HasPartOf(ToSlot(slotPos)))
                    {
                        m_editContext.slotArmorCounter[slotPos] += 1;
                    }
                }
                view2.Insert(*itBegin);
            }
        }
        view_remove_armors_in_outfit(editingOutfit);
    }

    auto OutfitEditPanel::IsFilterArmor(const Armor *armor) -> bool
    {
        if (m_editContext.filterPlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0)
        {
            return true;
        }
        std::string filterString = m_editContext.filterStringBuf.data();
        if (filterString.empty())
        {
            return false;
        }
        std::string armorName = armor->GetName();
        std::string modName;
        if (const auto *modFile = armor->GetFile(); modFile != nullptr)
        {
            modName.assign(modFile->GetFilename());
        }

        constexpr auto comparator = [](const char a, const char b) {
            return std::toupper(a) == std::toupper(b);
        };

        if (const auto itArmorName = std::ranges::search(armorName, filterString, comparator).begin();
            itArmorName != armorName.end())
        {
            return false;
        }
        if (const auto itModName = std::ranges::search(modName, filterString, comparator).begin();
            itModName != modName.end())
        {
            return false;
        }
        return true;
    }

    void OutfitEditPanel::OnAddArmor(const SosUiData::OutfitPair &wantEdit, Armor *armor)
    {
        if (wantEdit.second->IsConflictWith(armor))
        {
            m_ConflictArmorPopup.Open();
        }
        else
        {
            *this << m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
            m_editContext.armorView2.Remove(armor);
        }
    }

    void OutfitEditPanel::RenderPopups(const SosUiData::OutfitPair &wantEdit)
    {
        ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
        ImGui::PushFontSize(HintFontSize());
        if (m_ConflictArmorPopup.Render(m_editContext.selectedArmor))
        {
            *this << m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), m_editContext.selectedArmor);
            m_editContext.armorView2.Remove(m_editContext.selectedArmor);
        }

        if (m_DeleteArmorPopup.Render(m_editContext.selectedArmor))
        {
            *this << m_outfitService.DeleteArmor(wantEdit.first, wantEdit.second->GetName(),
                                                 m_editContext.selectedArmor);
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