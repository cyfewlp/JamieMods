//
// Created by jamie on 2025/4/6.
//

#include "SosOutfit.h"

#include "ImGuiUtil.h"
#include "PapyrusEvent.h"
#include "SosUiData.h"

#include <imgui.h>

namespace LIBC_NAMESPACE_DECL
{
    void SosOutfit::Render()
    {
        RenderArmorList();
        RenderEditPanel();
    }

    void SosOutfit::RenderArmorList()
    {
        if (m_armors.empty())
        {
            ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
            ImGuiUtil::Text("$SosGui_EmptyHint{$ARMOR}");
            ImGui::PopFontSize();
            return;
        }

        m_armorListTable.rows = m_armors.size();
        RenderTable(m_armorListTable, [this](int rowIdx) {
            auto armor = m_armors.at(rowIdx);
            ImGui::TableNextColumn();
            auto underlying = static_cast<uint32_t>(armor->GetSlotMask());
            ImGuiUtil::Text(std::format("$SkyOutSys_BodySlot{}", underlying));

            ImGui::TableNextColumn();
            ImGui::Text("%s", armor->GetName());
        });
    }

    void SosOutfit::RenderEditPanel()
    {
        ImGui::BeginGroup();
        RenderAddPolicyByCandidates();
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        RenderEditPanelPolicy();
        ImGui::EndGroup();
    }

    void SosOutfit::RenderAddPolicyByCandidates()
    {
        static int  pageSize        = 20;
        static int  currentPage     = 0;
        static Slot preSelectedSlot = Slot::kNone;
        auto        selectedSlot    = RenderArmorSlotFilter();
        if (preSelectedSlot != selectedSlot)
        {
            currentPage     = 0;
            preSelectedSlot = selectedSlot;
            UpdateArmorCandidatesBySlot(preSelectedSlot);
        }
        auto &outfitCandidates = SosUiData::GetInstance().GetArmorCandidatesCopy();
        if (outfitCandidates.empty())
        {
            ImGuiUtil::TextScale("$SosGui_EmptyHint{$ARMOR}", 1.2F);
            return;
        }

        int  startIdx = currentPage * pageSize;
        auto begin    = outfitCandidates.begin() + startIdx;

        m_armorCandidatesTable.rows = pageSize;
        bool complete = RenderTable(m_armorCandidatesTable, [this, &begin, &outfitCandidates](uint32_t &rowIdx) {
            if (begin == outfitCandidates.end())
            {
                return false;
            }
            auto *armor = *begin;
            ImGui::TableNextColumn();
            ImGui::Text("%s", armor->GetName());

            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<uint32_t>(armor->GetSlotMask()));

            ImGui::TableNextColumn();
            if (ImGuiUtil::Button("$Add") && TryAddArmor(m_name, armor))
            {
                AddArmor(armor);
                begin = outfitCandidates.erase(begin);
            }
            else
            {
                ++begin;
            }
            ++rowIdx;
            return true;
        });

        ImGui::BeginDisabled(currentPage == 0);
        if (ImGuiUtil::Button("$SosGui_Table_PrevPage"))
        {
            currentPage -= 1;
        }
        ImGui::SameLine();
        ImGui::EndDisabled();

        ImGui::Text("%d", currentPage + 1);
        ImGui::SameLine();

        ImGui::BeginDisabled(!complete);
        if (ImGui::Button("$SosGui_Table_NextPage"))
        {
            currentPage += 1;
        }
        ImGui::EndDisabled();
    }

    void SosOutfit::RenderOutfitAddPolicyById(const std::string &outfitName, const bool &fFilterPlayable)
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
                PapyrusEvent::GetInstance().CallAddToOutfit(outfitName, armor);
            }
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }

    void SosOutfit::RenderEditPanelPolicy() const
    {
        static std::array outfitPolicy{ImGuiUtil::Translate("$SkyOutSys_OEdit_AddFromCarried"),
                                       ImGuiUtil::Translate("$SkyOutSys_OEdit_AddFromWorn"),
                                       ImGuiUtil::Translate("$SkyOutSys_OEdit_AddByID"),
                                       ImGuiUtil::Translate("$SkyOutSys_OEdit_AddFromList_Header")};
        int               idx         = 0;
        static int        selectedIdx = 0;
        bool              fSelected   = false;
        ImGui::NewLine();
        for (const auto &policy : outfitPolicy)
        {
            ImGui::PushID(idx);

            if (ImGui::RadioButton(policy.c_str(), idx == selectedIdx))
            {
                fSelected   = selectedIdx != idx;
                selectedIdx = idx;
            }
            ImGui::PopID();
            idx++;
        }
        static std::array<char, MAX_FILTER_ARMOR_NAME> filterStringBuf;
        static bool                                    fFilterPlayable = false;

        const auto selectedPolicy         = static_cast<OutfitAddPolicy>(selectedIdx);
        bool       shouldUpdateCandidates = fSelected;
        if (fSelected)
        {
            if (selectedPolicy == OutfitAddPolicy_AddByID)
            {
                filterStringBuf[0] = '\0';
            }
        }

        if (ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &fFilterPlayable))
        {
            shouldUpdateCandidates = true;
        }
        if (selectedPolicy == OutfitAddPolicy_AddByID)
        {
            RenderOutfitAddPolicyById(m_name, fFilterPlayable);
            return;
        }

        // filter armor name and mod name
        ImGuiUtil::InputText("$SkyOutSys_OEdit_AddFromList_Filter_Name", filterStringBuf);
        if (!ImGui::GetIO().WantTextInput && ImGui::IsItemDeactivated())
        {
            shouldUpdateCandidates = true;
        }
        if (shouldUpdateCandidates)
        {
            UpdateArmorCandidates(filterStringBuf.data(), fFilterPlayable, selectedPolicy);
        }
    }

    bool SosOutfit::TryAddArmor(const std::string &outfitName, const Armor *a_armor) const
    {
        // get outfit armors
        if (a_armor == nullptr)
        {
            return false; // don't call get armors
        }
        bool conflict = false;
        for (const auto &armor : m_armors)
        {
            auto cSlots = armor->bipedModelData.bipedObjectSlots;
            if (cSlots.any(a_armor->GetSlotMask()))
            {
                conflict = true;
                break;
            }
        }
        if (!conflict)
        {
            PapyrusEvent::GetInstance().CallAddToOutfit(outfitName, a_armor);
            return true;
        }
        ImGuiUtil::OpenPopup("$SosGui_ArmorConflict");
        bool swapArmors = false;
        if (ImGuiUtil::BeginPopupModal("$SosGui_ArmorConflict"))
        {
            ImGuiUtil::TextWrapped("$SkyOutSys_Confirm_BodySlotConflict_Text");
            if (ImGuiUtil::Button("$SkyOutSys_Confirm_BodySlotConflict_Yes"))
            {
                swapArmors = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine(0, 12.0F);
            if (ImGuiUtil::Button("$SkyOutSys_Confirm_BodySlotConflict_No"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (swapArmors)
        {
            PapyrusEvent::GetInstance().CallSwapArmor(outfitName, a_armor);
        }
        return swapArmors;
    }

    constexpr int BODY_SLOT_MIN = 29;

    auto SosOutfit::RenderArmorSlotFilter() -> Slot
    {
        static int selectedIdx = 0;

        if (ImGui::BeginCombo("##ArmorSlotFilter", ARMOR_SLOT_NAMES.at(selectedIdx), ImGuiComboFlags_WidthFitPreview))
        {
            for (int idx = 0; idx <= 32; ++idx)
            {
                ImGui::PushID(idx);
                bool isSelected = selectedIdx == idx;
                if (ImGuiUtil::Selectable(std::format("$SkyOutSys_BodySlot{}", idx + BODY_SLOT_MIN), isSelected))
                {
                    selectedIdx = idx;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        return selectedIdx == 0 ? Slot::kNone : static_cast<Slot>(1 << (selectedIdx - 1));
    }

    void SosOutfit::UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                          OutfitAddPolicy policy)
    {
        switch (policy)
        {
            case OutfitAddPolicy_AddFromCarried:
            case OutfitAddPolicy_AddFromWorn:
                PapyrusEvent::GetInstance().CallGetActorArmors(RE::PlayerCharacter::GetSingleton(), policy);
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

    void SosOutfit::UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable)
    {
        auto       data   = RE::TESDataHandler::GetSingleton();
        auto      &list   = data->GetFormArray(RE::FormType::Armor);
        const auto size   = list.size();
        auto      &uiData = SosUiData::GetInstance();
        uiData.SetArmorCandidates({});
        auto &candidates = uiData.GetArmorCandidates();

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
        uiData.ResetArmorCandidatesCopy();
    }

    void SosOutfit::UpdateArmorCandidatesBySlot(Slot slot)
    {
        const auto &candidates = SosUiData::GetInstance().GetArmorCandidates();
        auto       &copy       = SosUiData::GetInstance().GetArmorCandidatesCopy();
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

    void SosOutfit::FilterArmorCandidates(const std::string_view &filterString, std::vector<Armor *> &armorCandidates)
    {
        if (filterString.empty())
        {
            return;
        }

        for (auto armorIter = armorCandidates.begin(); armorIter != armorCandidates.end();)
        {
            auto *armor = *armorIter;
            if (IsFilterArmor(filterString, armor))
            {
                armorIter = armorCandidates.erase(armorIter);
            }
            else
            {
                ++armorIter;
            }
        }
    }

    auto SosOutfit::IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool
    {
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
}