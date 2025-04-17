#include "gui/OutfitEditPanel.h"

#include "ImGuiUtil.h"
#include "SosDataType.h"
#include "SosUiData.h"
#include "Translation.h"
#include "common/config.h"
#include "data/PagedArmorList.h"
#include "data/SosUiOutfit.h"
#include "gui/Popup.h"
#include "gui/widgets.h"
#include "imgui.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/F/FormTypes.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cctype>
#include <cstdint>
#include <format>
#include <mutex>
#include <stdlib.h>
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
            *this << m_dataCoordinator.RequestRenameOutfit(wantEdit, outfitNameBuf.data());
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

        // render all in order but skip empty slot
        auto page = m_uiData.GetArmorCandidates().GetCurrentPage();

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
            auto slotPolicies = wantEdit.second->GetSlotPolicies();
            for (uint32_t slotIdx = 0; slotIdx < SLOT_COUNT; ++slotIdx)
            {
                const auto &armor = wantEdit.second->GetArmorAt(slotIdx);
                if (!showAllSlots && armor == nullptr) { continue; }

                ImGuiUtil::PushIdGuard idHolder(slotIdx);

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); // number column
                ImGui::Text("%d", slotIdx);

                ImGui::TableNextColumn(); // slot name column
                const bool  isSelected = selectedIdx == slotIdx;
                auto        name       = std::format("$SkyOutSys_BodySlot{}", slotIdx + SOS_SLOT_OFFSET);
                static auto flags      = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;
                if (ImGuiUtil::Selectable(name, isSelected, flags)) { selectedIdx = isSelected ? -1 : slotIdx; }
                if (armor != nullptr) { HighlightConflictArmor(armor); }

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
        auto        policyNameKey = wantEdit.second->GetSlotPolicies().at(slotIdx);
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
                    *this << m_dataCoordinator.RequestSetOutfitSlotPolicy(wantEdit, slotIdx, policy);
                }
                ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
            }
            ImGui::EndCombo();
        }
    }

    void OutfitEditPanel::RenderEditPanel(const SosUiData::OutfitPair &wantEdit)
    {
        if (RenderArmorSlotFilter()) { UpdateArmorCandidatesBySlot(wantEdit, m_editContext.selectedFilterSlot.get()); }

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

    auto OutfitEditPanel::RenderArmorSlotFilter() -> bool
    {
        static bool                         prevCheckAllSlot = false;
        static std::unordered_set<uint32_t> selectedSlotPos;

        std::call_once(init_slot_name_flag, init_slot_name);

        bool checked                = false;
        m_editContext.newFilterSlot = SLOT_COUNT;
        if (ImGuiUtil::BeginCombo("$SosGui_Combo_SlotFilter", nullptr))
        {
            for (uint8_t idx = 0; idx < SLOT_COUNT; ++idx)
            {
                ImGuiUtil::PushIdGuard idGuard(idx);
                if (ImGui::Selectable(slotNames[idx].c_str(), false))
                {
                    selectedSlotPos.insert(idx);
                    m_editContext.selectedFilterSlot.set(static_cast<Slot>(1 << idx));
                    checked                     = true;
                    m_editContext.checkSlotAll  = false;
                    m_editContext.newFilterSlot = idx;
                }
            }
            ImGui::EndCombo();
        }

        const auto  contentWidth = ImGui::GetContentRegionAvail().x;
        auto        cursorPosX   = 0.0F;
        std::string name =
            std::format("All({}/{})##AllSlot", m_uiData.GetCandidateArmorCount(), m_uiData.GetAllArmorCount());
        ImGui::Checkbox(name.c_str(), &m_editContext.checkSlotAll);
        const auto &style = ImGui::GetStyle();
        cursorPosX += ImGui::GetItemRectSize().x + style.ItemSpacing.x;
        ImGui::SameLine();

        for (auto itBegin = selectedSlotPos.begin(); itBegin != selectedSlotPos.end();)
        {
            const auto slot          = static_cast<Slot>(1 << *itBegin);
            bool       alwaysChecked = true;
            name = std::format("{}({})", slotNames[*itBegin], m_editContext.slotArmorCounter[*itBegin]);
            if (ImGui::Checkbox(name.c_str(), &alwaysChecked))
            {
                itBegin = selectedSlotPos.erase(itBegin);
                m_editContext.selectedFilterSlot.reset(slot);
                checked = true;
            }
            else { ++itBegin; }
            const auto itemX = ImGui::GetItemRectSize().x;
            if (const auto nextPos = cursorPosX + itemX + style.ItemSpacing.x * 2.0F; nextPos <= contentWidth)
            {
                ImGui::SameLine();
                cursorPosX = nextPos;
            }
            else { cursorPosX = 0.0F; }
        }
        if (prevCheckAllSlot != m_editContext.checkSlotAll && m_editContext.checkSlotAll)
        {
            m_editContext.selectedFilterSlot.set(static_cast<Slot>(UINT32_MAX));
            checked = true;
        }
        prevCheckAllSlot = m_editContext.checkSlotAll;
        ImGui::NewLine();
        return checked;
    }

    void OutfitEditPanel::RenderArmorCandidates(const SosUiData::OutfitPair &wantEdit)
    {
        auto &armorCandidates = m_uiData.GetArmorCandidates();

        if (armorCandidates.IsEmpty())
        {
            ImGui::PushFontSize(HintFontSize());
            ImGuiUtil::Text(armorCandidates.IsEmpty() ? "$SosGui_EmptyHint{$ARMOR}" : "");
            ImGui::PopFontSize();
            return;
        }

        ImGui::BeginDisabled(!armorCandidates.HasPrevPage());
        if (ImGuiUtil::Button("$SosGui_Table_PrevPage")) { armorCandidates.PrevPage(); }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Text("%d/%d", armorCandidates.GetPageIndex(), armorCandidates.GetPageCount());

        ImGui::SameLine();
        ImGui::BeginDisabled(!armorCandidates.HasNextPage());
        if (ImGuiUtil::Button("$SosGui_Table_NextPage")) { armorCandidates.NextPage(); }
        ImGui::EndDisabled();

        ImGui::SameLine();
        int curPageSize = armorCandidates.GetPageSize();
        if (ImGuiUtil::SliderInt("$SosGui_Table_PageSize", &curPageSize, 1, 500))
        {
            armorCandidates.SetPageSize(static_cast<uint16_t>(curPageSize));
        }

        if (!m_armorCandidatesTable.Begin()) { return; }

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
                armorCandidates.SetSortDirection(direction == ImGuiSortDirection_Ascending);
                sortSpecs->SpecsDirty = false;
            }
        }

        int         count         = 0;
        static bool requireAdd    = false;
        static bool requireAddAll = false;

        // clang-format off
        auto *multiSelectionIo = m_editContext.candidateSelection
                .NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid()
                .Begin(armorCandidates.GetPageSize());
        // clang-format on
        m_editContext.candidateSelection.ApplyRequests(multiSelectionIo);

        requireAddAll = false;
        armorCandidates.ForEachPage([this, &count](const size_t index, Armor *armor) {
            ImGuiUtil::PushIdGuard idGuard(index);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%zu", index);

            ImGui::TableNextColumn(); // armor name column

            bool isSelected = m_editContext.candidateSelection.Contains(static_cast<ImGuiID>(index));
            ImGui::SetNextItemSelectionUserData(index);
            ImGui::Selectable(armor->GetName(), isSelected, SELECTABLE_SPAN_FLAGS);
            if (isSelected) { m_editContext.candidateSelectedSlot.set(armor->GetSlotMask()); }
            else { m_editContext.candidateSelectedSlot.reset(armor->GetSlotMask()); }

            CandidateContextMenu(requireAddAll);

            ImGui::TableNextColumn(); // formId column
            ImGui::Text("%s", std::format("{:#010X}", armor->GetFormID()).c_str());

            ImGui::TableNextColumn(); // mod name column
            std::string_view modName = "";
            if (const auto modFile = armor->GetFile(); modFile != nullptr) { modName = modFile->GetFilename(); }
            ImGui::Text("%s", modName.data());

            ImGui::TableNextColumn(); // Action
            if (ImGuiUtil::Button("$Add"))
            {
                m_editContext.selectedArmor = armor;
                requireAdd                  = true;
            }
            ++count;
        });
        ImGui::EndTable();
        multiSelectionIo = ImGui::EndMultiSelect();
        m_editContext.candidateSelection.ApplyRequests(multiSelectionIo);

        // handle multi-selection
        if (requireAddAll && m_editContext.candidateSelection.Size > 0)
        {
            auto page = armorCandidates.GetCurrentPage();

            uint16_t index = 0;
            for (auto itBegin = page.begin(); itBegin != page.end(); ++index, ++itBegin)
            {
                if (m_editContext.candidateSelection.Contains(index))
                {
                    *this << m_dataCoordinator.RequestAddArmor(wantEdit, *itBegin);
                }
            }
            m_editContext.candidateSelection.Clear();
        }

        if (requireAdd)
        {
            requireAdd = false;
            OnAddArmor(wantEdit, m_editContext.selectedArmor);
        }
    }

    void OutfitEditPanel::CandidateContextMenu(bool &acceptAddAll)
    {
        if (!ImGui::BeginPopupContextItem()) { return; }
        if (ImGuiUtil::MenuItem("$SosGui_ContextMenu_AddAllArmor")) { acceptAddAll = true; }
        ImGui::EndPopup();
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
            if (selectedPolicy == OutfitAddPolicy_AddByID) { m_editContext.filterStringBuf[0] = '\0'; }
        }

        if (ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &m_editContext.filterPlayable))
        {
            shouldUpdateCandidates = true;
        }
        if (selectedPolicy == OutfitAddPolicy_AddByID)
        {
            RenderOutfitAddPolicyById(wantEdit, m_editContext.filterPlayable);
            return;
        }

        // filter armor name and mod name
        ImGuiUtil::InputText("$SkyOutSys_OEdit_AddFromList_Filter_Name", m_editContext.filterStringBuf);
        if (ImGui::IsItemDeactivatedAfterEdit()) { shouldUpdateCandidates = true; }
        if (shouldUpdateCandidates) { UpdateArmorCandidates(wantEdit, m_editContext.filterPlayable, selectedPolicy); }
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
            if (ImGuiUtil::Button("$Add")) { *this << m_dataCoordinator.RequestAddArmor(wantEdit, armor); }
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }

    void OutfitEditPanel::UpdateArmorCandidates(const SosUiData::OutfitPair &wantEdit, const bool mustBePlayable,
                                                const OutfitAddPolicy policy)
    {
        switch (policy)
        {
            case OutfitAddPolicy_AddFromCarried:
                *this << m_dataCoordinator.RequestGetArmorsByCarried();
                break;

            case OutfitAddPolicy_AddFromWorn:
                *this << m_dataCoordinator.RequestGetArmorsByWorn();
                break;

            case OutfitAddPolicy_AddByID:
                break;

            case OutfitAddPolicy_AddAny:
                UpdateArmorCandidatesForAny(wantEdit, mustBePlayable);
                break;

            default:
                break;
        }
        m_editContext.checkSlotAll       = true;
        m_editContext.selectedFilterSlot = Slot::kNone;
    }

    void OutfitEditPanel::UpdateArmorCandidatesForAny(const SosUiData::OutfitPair &wantEdit,
                                                      const bool                   mustBePlayable) const
    {
        const auto data       = RE::TESDataHandler::GetSingleton();
        auto      &list       = data->GetFormArray(RE::FormType::Armor);
        auto      &candidates = m_uiData.GetArmorCandidates();
        candidates.Clear();

        for (const auto &form : list)
        {
            if (form == nullptr) { continue; }
            auto *armor = form->As<Armor>();
            if (armor == nullptr || armor->templateArmor != nullptr) { continue; }
            if (std::string_view name = armor->GetName(); name.empty()) { continue; }
            if (mustBePlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0) { continue; }
            if (!IsFilterArmor(m_editContext.filterStringBuf.data(), armor)) { candidates.Insert(armor, true); }
        }
        for (size_t slotPos = 0; slotPos < SosUiOutfit::SLOT_COUNT; slotPos++)
        {
            auto *armor = wantEdit.second->GetArmorAt(slotPos);
            candidates.Insert(armor, false);
        }
    }

    void OutfitEditPanel::UpdateArmorCandidatesBySlot(const SosUiData::OutfitPair &wantEdit, Slot slot)
    {
        auto &candidates = m_uiData.GetArmorCandidates();
        candidates.RestoreUnusedArmors();

        if (m_editContext.newFilterSlot != SLOT_COUNT)
        {
            m_editContext.slotArmorCounter[m_editContext.newFilterSlot] = 0;
        }

        std::vector<PagedArmorList::ArmorWrap> filterArmors;
        for (auto itBegin = candidates.begin(); itBegin != candidates.end(); ++itBegin)
        {
            if (!itBegin->HasAnyPartOf(slot)) { filterArmors.push_back(*itBegin); }
            if (itBegin->HasAnyPartOf(ToSlot(m_editContext.newFilterSlot)))
            {
                m_editContext.slotArmorCounter[m_editContext.newFilterSlot] += 1;
            }
        }
        candidates.Insert(filterArmors, false);
        for (size_t slotPos = 0; slotPos < SosUiOutfit::SLOT_COUNT; slotPos++)
        {
            auto *armor = wantEdit.second->GetArmorAt(slotPos);
            candidates.Insert(armor, false);
        }
    }

    auto OutfitEditPanel::IsFilterArmor(const std::string_view &filterString, const Armor *armor) -> bool
    {
        if (filterString.empty()) { return false; }
        std::string armorName = armor->GetName();
        std::string modName;
        if (const auto *modFile = armor->GetFile(); modFile != nullptr) { modName.assign(modFile->GetFilename()); }

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
        if (wantEdit.second->IsConflictWith(armor)) { m_ConflictArmorPopup.Open(); }
        else
        {
            *this << m_dataCoordinator.RequestAddArmor(wantEdit, armor);
            m_uiData.MarkArmorIsUnused(armor);
        }
    }

    void OutfitEditPanel::RenderPopups(const SosUiData::OutfitPair &wantEdit)
    {
        ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
        ImGui::PushFontSize(HintFontSize());
        if (m_ConflictArmorPopup.Render(m_editContext.selectedArmor))
        {
            *this << m_dataCoordinator.RequestAddArmor(wantEdit, m_editContext.selectedArmor);
            m_uiData.MarkArmorIsUnused(m_editContext.selectedArmor);
        }

        if (m_DeleteArmorPopup.Render(m_editContext.selectedArmor))
        {
            *this << m_dataCoordinator.RequestDeleteArmor(wantEdit, m_editContext.selectedArmor);
        }
        m_slotPolicyHelp.Render();

        ImGui::PopFontSize();
        ImGui::PopStyleVar();
        if (m_ConflictArmorPopup.IsLastClosed() || m_DeleteArmorPopup.IsLastClosed())
        {
            m_editContext.selectedArmor = nullptr;
        }
    }
}