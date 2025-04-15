#include "gui/OutfitEditPanel.h"

#include "ImGuiUtil.h"
#include "SosDataType.h"
#include "SosUiData.h"
#include "Translation.h"
#include "common/config.h"
#include "data/PagedArmorList.h"
#include "data/SosUiOutfit.h"
#include "gui/Popup.h"
#include "imgui.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/F/FormTypes.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <algorithm>
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
        auto flags = ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushID(this);
        if (ImGui::Begin(m_windowTitle.c_str(), &m_fShowOutfitWindow, flags))
        {
            RenderPopups(wantEdit);
            RenderProperties(wantEdit);
            RenderArmorList(wantEdit);
            RenderEditPanel(wantEdit);
        }
        ImGui::End();
        ImGui::PopID();
        return !m_fShowOutfitWindow;
    }

    void OutfitEditPanel::UpdateWindowTitle(const std::string &outfitName)
    {
        m_windowTitle = Translation::TranslateIgnoreNested("$SkyOutSys_MCMHeader_OutfitEditor{}");
        auto pos      = m_windowTitle.find("{}");
        if (pos != std::string::npos)
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
        static int selectedIdx = -1;
        if (m_armorListTable.Begin())
        {
            m_armorListTable.HeadersRow();
            for (int slotIdx = 0; slotIdx < SLOT_COUNT; ++slotIdx)
            {
                const auto &armor = wantEdit.second->GetArmorAt(slotIdx);
                if (armor == nullptr)
                {
                    continue;
                }

                ImGuiUtil::PushIdGuard idHolder(slotIdx);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                bool        isSelected = selectedIdx == slotIdx;
                auto        name       = std::format("$SkyOutSys_BodySlot{}", slotIdx + SOS_SLOT_OFFSET);
                static auto flags      = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;
                if (ImGuiUtil::Selectable(name, isSelected, flags))
                {
                    selectedIdx = isSelected ? -1 : slotIdx;
                }

                ImGui::TableNextColumn();
                ImGui::Text("%s", armor->GetName());

                ImGui::TableNextColumn();
                if (ImGuiUtil::Button("$Delete"))
                {
                    m_selectedArmor = armor;
                    m_DeleteArmorPopup.Open();
                }
            }
            ImGui::EndTable();
        }
    }

    void OutfitEditPanel::RenderEditPanel(const SosUiData::OutfitPair &wantEdit)
    {
        if (RenderArmorSlotFilter())
        {
            UpdateArmorCandidatesBySlot(wantEdit, m_selectedFilterSlot.get());
        }

        ImGuiStyle &style     = ImGui::GetStyle();
        float       halfWidth = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2;
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
        static bool                         checkAllSlot     = false;
        static bool                         prevCheckAllSlot = false;
        static std::unordered_set<uint32_t> selectedSlotPos;

        std::call_once(init_slot_name_flag, init_slot_name);

        bool checked = false;
        if (ImGui::BeginCombo("##AddArmorSlotToFilter", nullptr))
        {
            for (uint32_t idx = 0; idx < SLOT_COUNT; ++idx)
            {
                ImGuiUtil::PushIdGuard idGuard(idx);
                if (ImGui::Selectable(slotNames[idx].c_str(), false))
                {
                    selectedSlotPos.insert(idx);
                    m_selectedFilterSlot.set(static_cast<Slot>(1 << idx));
                    checked      = true;
                    checkAllSlot = false;
                }
            }
            ImGui::EndCombo();
        }

        auto contentWidth = ImGui::GetContentRegionAvail().x;
        auto cursorPosX   = 0.0F;
        ImGui::Checkbox("All##AllSlot", &checkAllSlot);
        auto &style = ImGui::GetStyle();
        cursorPosX += ImGui::GetItemRectSize().x + style.ItemSpacing.x;
        ImGui::SameLine();

        for (auto itBegin = selectedSlotPos.begin(); itBegin != selectedSlotPos.end();)
        {
            Slot slot          = static_cast<Slot>(1 << *itBegin);
            bool alwaysChecked = true;
            if (ImGui::Checkbox(slotNames[*itBegin].c_str(), &alwaysChecked))
            {
                itBegin = selectedSlotPos.erase(itBegin);
                m_selectedFilterSlot.reset(slot);
                checked = true;
            }
            else
            {
                ++itBegin;
            }
            auto itemX   = ImGui::GetItemRectSize().x;
            auto nextPos = cursorPosX + itemX + style.ItemSpacing.x * 2.0F;
            if (nextPos <= contentWidth)
            {
                ImGui::SameLine();
                cursorPosX = nextPos;
            }
            else
            {
                cursorPosX = 0.0F;
            }
        }
        if ((prevCheckAllSlot != checkAllSlot) && checkAllSlot)
        {
            m_selectedFilterSlot.set(static_cast<Slot>(UINT32_MAX));
            prevCheckAllSlot = checkAllSlot;
            checked          = true;
        }
        ImGui::NewLine();
        return checked;
    }

    void OutfitEditPanel::RenderArmorCandidates(const SosUiData::OutfitPair &wantEdit)
    {
        auto &armorCandidates = m_uiData.GetArmorCandidates();

        ImGui::PushFontSize(HintFontSize());
        ImGuiUtil::Text(armorCandidates.IsEmpty() ? "$SosGui_EmptyHint{$ARMOR}" : "");
        ImGui::PopFontSize();

        if (armorCandidates.IsEmpty() || !m_armorCandidatesTable.Begin())
        {
            return;
        }
        ImGui::PushID("HeadersRow");
        m_armorCandidatesTable.Column(0)
            .DefaultSort()
            .Setup()
            .Column(1)
            .WidthFixed()
            .NoSort()
            .Setup()
            .Column(2)
            .NoSort()
            .Setup()
            .Column(3)
            .WidthFixed()
            .NoSort()
            .Setup()
            .Column(4)
            .WidthFixed()
            .NoSort()
            .Setup();
        ImGui::TableHeadersRow();
        ImGui::PopID();

        if (auto *sortSpecs = ImGui::TableGetSortSpecs(); (sortSpecs != nullptr))
        {
            if (sortSpecs->SpecsDirty)
            {
                assert(sortSpecs->SpecsCount == 1);
                auto direction = sortSpecs->Specs[0].SortDirection;
                armorCandidates.SetSortDirection(direction == ImGuiSortDirection_Ascending);
                sortSpecs->SpecsDirty = false;
            }
        }

        int         count       = 0;
        static int  selectedIdx = -1;
        static bool requireAdd  = false;
        armorCandidates.ForEachPage([this, &count](size_t index, Armor *armor) {
            ImGuiUtil::PushIdGuard idGuard(index);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            bool isSelected = selectedIdx == index;
            if (ImGui::Selectable(armor->GetName(), isSelected, SELECTABLE_SPAN_FLAGS))
            {
                selectedIdx = isSelected ? -1 : index;
            }
            ImGui::TableNextColumn();
            ImGui::Text("%s", std::format("{:#010X}", armor->GetFormID()).c_str());

            ImGui::TableNextColumn();
            std::string_view modName = "";
            if (auto modFile = armor->GetFile(); (modFile != nullptr))
            {
                modName = modFile->GetFilename();
            }
            ImGui::Text("%s", modName.data());

            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<uint32_t>(armor->GetSlotMask()));

            ImGui::TableNextColumn();
            if (ImGuiUtil::Button("$Add"))
            {
                m_selectedArmor = armor;
                requireAdd      = true;
            }
            ++count;
        });
        ImGui::EndTable();

        if (requireAdd)
        {
            requireAdd = false;
            OnAddArmor(wantEdit, m_selectedArmor);
        }

        // ImGui::BeginDisabled(currentPage == 0);
        ImGui::BeginDisabled(!armorCandidates.HasPrevPage());
        if (ImGuiUtil::Button("$SosGui_Table_PrevPage"))
        {
            armorCandidates.PrevPage();
        }
        ImGui::SameLine();
        ImGui::EndDisabled();

        ImGui::Text("%d/%d", armorCandidates.GetPageIndex(), armorCandidates.GetPageCount());
        ImGui::SameLine();

        // ImGui::BeginDisabled(count != pageSize);
        ImGui::BeginDisabled(!armorCandidates.HasNextPage());
        if (ImGuiUtil::Button("$SosGui_Table_NextPage"))
        {
            armorCandidates.NextPage();
        }
        ImGui::EndDisabled();
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
            ImGuiUtil::PushIdGuard idguard(idx);

            if (ImGui::RadioButton(policy.c_str(), (idx == m_armorAddPolicy)))
            {
                fSelected        = m_armorAddPolicy != idx;
                m_armorAddPolicy = idx;
            }
            idx++;
        }

        const auto selectedPolicy         = static_cast<OutfitAddPolicy>(m_armorAddPolicy);
        bool       shouldUpdateCandidates = fSelected;
        if (fSelected)
        {
            if (selectedPolicy == OutfitAddPolicy_AddByID)
            {
                m_filterStringBuf[0] = '\0';
            }
        }

        if (ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &m_fFilterPlayable))
        {
            shouldUpdateCandidates = true;
        }
        if (selectedPolicy == OutfitAddPolicy_AddByID)
        {
            RenderOutfitAddPolicyById(wantEdit, m_fFilterPlayable);
            return;
        }

        // filter armor name and mod name
        ImGuiUtil::InputText("$SkyOutSys_OEdit_AddFromList_Filter_Name", m_filterStringBuf);
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            shouldUpdateCandidates = true;
        }
        if (shouldUpdateCandidates)
        {
            UpdateArmorCandidates(wantEdit, m_fFilterPlayable, selectedPolicy);
        }
    }

    void OutfitEditPanel::RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit,
                                                    const bool                  &fFilterPlayable) 
    {
        static std::array<char, 32> formIdBuf;
        static char                *pEnd{};

        ImGui::PushID("AddByFormId");
        ImGui::Text("0x");
        ImGui::SameLine();
        ImGui::InputText("##InputArmorId", formIdBuf.data(), formIdBuf.size(),
                         ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal);
        auto   formId = std::strtoul(formIdBuf.data(), &pEnd, 16);
        Armor *armor  = nullptr;
        if ((*pEnd == 0) && ((armor = RE::TESForm::LookupByID<Armor>(formId)) != nullptr))
        {
            ImGui::Text("%s", armor->GetName());
            ImGui::SameLine();
            ImGui::BeginDisabled(fFilterPlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0);
            if (ImGuiUtil::Button("$Add"))
            {
                *this << m_dataCoordinator.RequestAddArmor(wantEdit, armor);
            }
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }

    void OutfitEditPanel::UpdateArmorCandidates(const SosUiData::OutfitPair &wantEdit, bool mustBePlayable,
                                                OutfitAddPolicy policy)
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
        m_selectedFilterSlot = Slot::kNone;
    }

    void OutfitEditPanel::UpdateArmorCandidatesForAny(const SosUiData::OutfitPair &wantEdit, bool mustBePlayable)
    {
        auto  data       = RE::TESDataHandler::GetSingleton();
        auto &list       = data->GetFormArray(RE::FormType::Armor);
        auto &candidates = m_uiData.GetArmorCandidates();

        candidates.Clear();
        for (const auto &form : list)
        {
            if ((form == nullptr) || (form->formType != RE::FormType::Armor))
            {
                continue;
            }
            auto *armor = form->As<Armor>();
            if ((armor == nullptr) || (armor->templateArmor != nullptr))
            {
                continue;
            }
            if (std::string_view name = armor->GetName(); name.empty())
            {
                continue;
            }
            if (mustBePlayable && ((armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0))
            {
                continue;
            }
            if (!IsFilterArmor(m_filterStringBuf.data(), armor))
            {
                candidates.Insert(armor, true);
            }
        }
        for (size_t slotPos = 0; slotPos < SosUiOutfit::SLOT_COUNT; slotPos++)
        {
            auto *armor = wantEdit.second->GetArmorAt(slotPos);
            candidates.Insert(armor, false);
        }
    }

    void OutfitEditPanel::UpdateArmorCandidatesBySlot(const SosUiData::OutfitPair &wantEdit, Slot slot) const
    {
        auto &candidates = m_uiData.GetArmorCandidates();

        candidates.RestoreUnusedArmors();

        std::vector<PagedArmorList::ArmorWrap> filterArmors;
        for (auto itBegin = candidates.begin(); itBegin != candidates.end(); ++itBegin)
        {
            if (!itBegin->HasAnyPartOf(slot))
            {
                filterArmors.push_back(*itBegin);
            }
        }
        candidates.Insert(filterArmors, false);
        for (size_t slotPos = 0; slotPos < SosUiOutfit::SLOT_COUNT; slotPos++)
        {
            auto *armor = wantEdit.second->GetArmorAt(slotPos);
            candidates.Insert(armor, false);
        }
    }

    auto OutfitEditPanel::IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool
    {
        if (filterString.empty())
        {
            return false;
        }
        std::string armorName = armor->GetName();
        std::string modName;
        if (auto *modFile = armor->GetFile(); (modFile != nullptr))
        {
            modName.assign(modFile->GetFilename());
        }
        constexpr auto comparator = [](char a, char b) {
            return std::toupper(a) == std::toupper(b);
        };
        auto itArmorName =
            std::search(armorName.begin(), armorName.end(), filterString.begin(), filterString.end(), comparator);
        if (itArmorName != armorName.end())
        {
            return false;
        }
        auto itModName =
            std::search(modName.begin(), modName.end(), filterString.begin(), filterString.end(), comparator);
        if (itModName != modName.end())
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
            *this << m_dataCoordinator.RequestAddArmor(wantEdit, armor);
            m_uiData.MarkArmorIsUnused(armor);
        }
    }

    void OutfitEditPanel::RenderPopups(const SosUiData::OutfitPair &wantEdit)
    {
        ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
        if (m_ConflictArmorPopup.Render(m_selectedArmor))
        {
            *this << m_dataCoordinator.RequestAddArmor(wantEdit, m_selectedArmor);
            m_uiData.MarkArmorIsUnused(m_selectedArmor);
        }

        if (m_DeleteArmorPopup.Render(m_selectedArmor))
        {
            *this << m_dataCoordinator.RequestDeleteArmor(wantEdit, m_selectedArmor);
        }
        ImGui::PopStyleVar();
        if (m_ConflictArmorPopup.IsLastClosed() || m_DeleteArmorPopup.IsLastClosed())
        {
            m_selectedArmor = nullptr;
        }
    }
}