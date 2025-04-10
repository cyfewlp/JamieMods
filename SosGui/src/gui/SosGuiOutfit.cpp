#include "gui/SosGuiOutfit.h"

#include "ImGuiUtil.h"
#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "data/SosUiOutfit.h"
#include "gui/Popup.h"
#include "imgui.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/F/FormTypes.h>
#include <RE/RTTI.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESFullName.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cstdint>
#include <format>
#include <stdlib.h>
#include <string>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    constexpr auto SELECTABLE_SPAN_FLAGS = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;

    auto SosGuiOutfit::Render(SosUiOutfit &editingOutfit) -> bool
    {
        auto flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushID(this);
        if (ImGui::Begin(m_windowTitle.c_str(), &m_fShowOutfitWindow, flags))
        {
            RenderPopups(editingOutfit.GetName());
            RenderProperties(editingOutfit);
            RenderArmorList(editingOutfit);
            RenderEditPanel(editingOutfit);
        }
        ImGui::End();
        ImGui::PopID();
        return m_fShowOutfitWindow;
    }

    void SosGuiOutfit::UpdateWindowTitle(const std::string &outfitName)
    {
        m_windowTitle = Translation::TranslateIgnoreNested("$SkyOutSys_MCMHeader_OutfitEditor{}");
        auto pos      = m_windowTitle.find("{}");
        if (pos != std::string::npos)
        {
            m_windowTitle.replace(pos, 2, outfitName);
        }
    }

    constexpr int BODY_SLOT_MIN = 29;

    void SosGuiOutfit::RenderProperties(SosUiOutfit &editingOutfit)
    {
        static std::array<char, 256> outfitNameBuf;
        ImGuiUtil::InputText("##OutfitRenameInput", outfitNameBuf);
        ImGui::SameLine();
        ImGui::BeginDisabled(outfitNameBuf.at(0) == '\0');
        if (ImGuiUtil::Button("$SkyOutSys_OContext_Rename"))
        {
            m_dataCoordinator.RequestRenameOutfit(editingOutfit.GetName(), outfitNameBuf.data());
            UpdateWindowTitle(editingOutfit.GetName());
        }
        ImGui::EndDisabled();
    }

    void SosGuiOutfit::RenderArmorList(SosUiOutfit &editingOutfit)
    {
        if (editingOutfit.IsEmpty())
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
                const auto &armor = editingOutfit.GetArmorAt(slotIdx);
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

    void SosGuiOutfit::RenderEditPanel(SosUiOutfit &editingOutfit)
    {
        auto selectedSlot = RenderArmorSlotFilter();

        ImGuiStyle &style     = ImGui::GetStyle();
        float       halfWidth = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2;

        {
            ImGuiUtil::ChildGuard guard("##RenderArmorCandidates", {halfWidth, 0.0F});
            RenderArmorCandidates(editingOutfit, selectedSlot);
        }

        ImGui::SameLine();

        {
            ImGuiUtil::ChildGuard guard("##RenderEditPanelPolicy", {halfWidth, 0.0F});
            RenderEditPanelPolicy(editingOutfit);
        }
    }

    auto SosGuiOutfit::RenderArmorSlotFilter() -> Slot
    {
        static int selectedIdx = 0;

        if (ImGui::BeginCombo("##ArmorSlotFilter", ARMOR_SLOT_NAMES.at(selectedIdx)))
        {
            for (int idx = 0; idx <= 32; ++idx)
            {
                ImGuiUtil::PushIdGuard idGuard(idx);

                bool isSelected = selectedIdx == idx;
                if (ImGuiUtil::Selectable(std::format("$SkyOutSys_BodySlot{}", idx + BODY_SLOT_MIN), isSelected))
                {
                    selectedIdx = idx;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        return selectedIdx == 0 ? Slot::kNone : static_cast<Slot>(1 << (selectedIdx - 1));
    }

    void SosGuiOutfit::RenderArmorCandidates(SosUiOutfit &editingOutfit, Slot selectedSlot)
    {
        static int  pageSize        = 20;
        static int  currentPage     = 0;
        static Slot preSelectedSlot = Slot::kNone;
        if (preSelectedSlot != selectedSlot)
        {
            currentPage     = 0;
            preSelectedSlot = selectedSlot;
            UpdateArmorCandidatesBySlot(preSelectedSlot);
        }
        auto &outfitCandidates = m_uiData.GetArmorCandidatesCopy();
        ImGui::PushFontSize(HintFontSize());
        ImGuiUtil::Text(outfitCandidates.empty() ? "$SosGui_EmptyHint{$ARMOR}" : "");
        ImGui::PopFontSize();

        if (outfitCandidates.empty() || !m_armorCandidatesTable.Begin())
        {
            return;
        }

        int        count       = 0;
        int        startIdx    = currentPage * pageSize;
        static int selectedIdx = -1;
        auto       begin       = outfitCandidates.begin() + startIdx;
        m_armorCandidatesTable.HeadersRow();
        for (auto &armorIter = begin; count < pageSize && armorIter != outfitCandidates.end(); ++count)
        {
            ImGuiUtil::PushIdGuard idGuard(count);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            auto *armor      = *armorIter;
            bool  isSelected = selectedIdx == count;
            if (ImGui::Selectable(armor->GetName(), isSelected, SELECTABLE_SPAN_FLAGS))
            {
                selectedIdx = isSelected ? -1 : count;
            }

            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<uint32_t>(armor->GetSlotMask()));

            ImGui::TableNextColumn();
            if (ImGuiUtil::Button("$Add"))
            {
                if (editingOutfit.IsConflictWith(armor))
                {
                    m_selectedArmor = armor;
                    m_ConflictArmorPopup.Open();
                }
                else
                {
                    m_dataCoordinator.RequestAddArmor(editingOutfit.GetName(), armor);
                    armorIter = outfitCandidates.erase(armorIter);
                    continue;
                }
            }
            ++armorIter;
        }
        ImGui::EndTable();

        ImGui::BeginDisabled(currentPage == 0);
        if (ImGuiUtil::Button("$SosGui_Table_PrevPage"))
        {
            currentPage -= 1;
        }
        ImGui::SameLine();
        ImGui::EndDisabled();

        ImGui::Text("%d", currentPage + 1);
        ImGui::SameLine();

        ImGui::BeginDisabled(count != pageSize);
        if (ImGuiUtil::Button("$SosGui_Table_NextPage"))
        {
            currentPage += 1;
        }
        ImGui::EndDisabled();
    }

    void SosGuiOutfit::RenderEditPanelPolicy(SosUiOutfit &editingOutfit)
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

            if (ImGui::RadioButton(policy.c_str(), idx == m_armorAddPolicy))
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
            RenderOutfitAddPolicyById(editingOutfit, m_fFilterPlayable);
            return;
        }

        // filter armor name and mod name
        ImGuiUtil::InputText("$SkyOutSys_OEdit_AddFromList_Filter_Name", m_filterStringBuf);
        if (!ImGui::GetIO().WantTextInput && ImGui::IsItemDeactivated())
        {
            shouldUpdateCandidates = true;
        }
        if (shouldUpdateCandidates)
        {
            UpdateArmorCandidates(m_filterStringBuf.data(), m_fFilterPlayable, selectedPolicy);
        }
    }

    void SosGuiOutfit::RenderOutfitAddPolicyById(SosUiOutfit &editingOutfit, const bool &fFilterPlayable) const
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
        if (*pEnd == 0 && (armor = RE::TESForm::LookupByID<Armor>(formId)) != nullptr)
        {
            ImGui::Text("%s", armor->GetName());
            ImGui::SameLine();
            ImGui::BeginDisabled(fFilterPlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0);
            if (ImGuiUtil::Button("$Add"))
            {
                m_dataCoordinator.RequestAddArmor(editingOutfit.GetName(), armor);
            }
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }

    void SosGuiOutfit::UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                             OutfitAddPolicy policy)
    {
        switch (policy)
        {
            case OutfitAddPolicy_AddFromCarried:
                m_dataCoordinator.RequestGetArmorsByCarried();
                break;
            case OutfitAddPolicy_AddFromWorn:
                m_dataCoordinator.RequestGetArmorsByWorn();
                break;
            case OutfitAddPolicy_AddByID:
                break;
            case OutfitAddPolicy_AddAny:
                UpdateArmorCandidatesForAny(filterString, mustBePlayable);
                break;
            default:
                break;
        }
    }

    void SosGuiOutfit::UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable)
    {
        auto       data       = RE::TESDataHandler::GetSingleton();
        auto      &list       = data->GetFormArray(RE::FormType::Armor);
        const auto size       = list.size();
        auto      &candidates = m_uiData.GetArmorCandidates();
        candidates.clear();

        for (std::uint32_t idx = 0; idx < size; idx++)
        {
            auto *const form = list[idx];
            if (form != nullptr && form->formType != RE::FormType::Armor)
            {
                continue;
            }
            auto *armor = skyrim_cast<Armor *>(form);
            if (armor == nullptr || armor->templateArmor == nullptr)
            {
                continue;
            }
            if (mustBePlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0)
            {
                continue;
            }
            if (!IsFilterArmor(filterString, armor))
            {
                candidates.push_back(armor);
            }
        }
        m_uiData.ResetArmorCandidatesCopy();
    }

    void SosGuiOutfit::UpdateArmorCandidatesBySlot(Slot slot)
    {
        const auto &candidates = m_uiData.GetArmorCandidates();
        auto       &copy       = m_uiData.GetArmorCandidatesCopy();
        copy.clear();
        for (const auto &candidate : candidates)
        {
            if (slot != Slot::kNone && !candidate->HasPartOf(slot))
            {
                continue;
            }
            copy.push_back(candidate);
        }
    }

    auto SosGuiOutfit::IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool
    {
        if (filterString.empty())
        {
            return false;
        }
        std::string armorName;
        std::string modName;
        if (auto *fullName = skyrim_cast<RE::TESFullName *>(armor); fullName != nullptr)
        {
            armorName.assign(fullName->GetFullName());
        }
        if (auto *modFile = armor->GetFile(); modFile != nullptr)
        {
            modName.assign(modFile->GetFilename());
        }
        if (armorName.empty() || !armorName.contains(filterString) || !modName.contains(filterString))
        {
            return true;
        }
        return false;
    }

    void SosGuiOutfit::RenderPopups(const std::string &outfitName)
    {
        ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
        if (m_ConflictArmorPopup.Render(m_selectedArmor))
        {
            m_dataCoordinator.RequestAddArmor(outfitName, m_selectedArmor);
            m_uiData.DeleteCandidateArmor(m_selectedArmor);
        }

        if (m_DeleteArmorPopup.Render(m_selectedArmor))
        {
            m_dataCoordinator.RequestDeleteArmor(outfitName, m_selectedArmor);
        }
        ImGui::PopStyleVar();
        if (m_ConflictArmorPopup.IsLastClosed() || m_DeleteArmorPopup.IsLastClosed())
        {
            m_selectedArmor = nullptr;
        }
    }
}