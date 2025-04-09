//
// Created by jamie on 2025/4/6.
//

#include "SosOutfit.h"

#include "ImGuiUtil.h"
#include "PapyrusEvent.h"
#include "SosUiData.h"
#include "imgui.h"

#include <vector>
#include <format>
#include <array>

namespace LIBC_NAMESPACE_DECL
{
    constexpr auto SELECTABLE_SPAN_FLAGS = ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;

    void SosOutfit::ConfirmPopup::Open(Armor *data, const std::function<void(Armor *)> &callback)
    {
        ImGui::OpenPopup(id);
        this->data      = data;
        this->onConfirm = callback;
    }

    void SosOutfit::ConfirmPopup::Close()
    {
        ImGui::CloseCurrentPopup();
        if (onConfirm != nullptr && isConfirm)
        {
            onConfirm(data);
        }
        data      = nullptr;
        isOpen    = false;
        onConfirm = nullptr;
    }

    void SosOutfit::ConfirmPopup::Render(const std::string_view &message)
    {
        this->id = ImGui::GetID(name.c_str());
        if (ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            {
                constexpr std::string_view delim{"\n"};
                std::ranges::split_view    splitView{message, delim};
                const auto                &contentSize = ImGui::GetContentRegionAvail();
                for (const auto &lineView : splitView)
                {
                    auto line     = std::string_view(lineView);
                    auto textSize = ImGui::CalcTextSize(line.data());
                    if (contentSize.x > textSize.x)
                    {
                        ImGui::SetCursorPosX((contentSize.x - textSize.x) * 0.5F);
                    }
                    ImGui::TextWrapped("%s", line.data());
                }
            }
            if (ImGuiUtil::Button("$SkyOutSys_Confirm_BodySlotConflict_Yes"))
            {
                this->isConfirm = true;
                Close();
            }
            ImGui::SameLine(0, 12.0F);
            if (ImGuiUtil::Button("$SkyOutSys_Confirm_BodySlotConflict_No"))
            {
                Close();
            }
            ImGui::EndPopup();
        }
    }

    void SosOutfit::UpdateWindowTitle()
    {
        m_windowTitle = Translation::TranslateIgnoreNested("$SkyOutSys_MCMHeader_OutfitEditor{}");
        auto pos      = m_windowTitle.find("{}");
        if (pos != std::string::npos)
        {
            m_windowTitle.replace(pos, 2, m_name);
        }
    }

    auto SosOutfit::Render() -> bool
    {
        auto flags = ImGuiWindowFlags_NoSavedSettings;
        ImGui::PushID(this);
        ImGui::Begin(m_windowTitle.c_str(), &m_fShowOutfitWindow, flags);
        RenderProperties();
        RenderArmorList();
        RenderEditPanel();
        ImGui::End();
        ImGui::PopID();
        return m_fShowOutfitWindow;
    }

    void SosOutfit::AddArmor(Armor *armor)
    {
        auto mask = static_cast<uint32_t>(armor->GetSlotMask());
        m_slotMask.set(armor->GetSlotMask());
        for (uint32_t slotIdx = 0; slotIdx < SLOT_COUNT; ++slotIdx)
        {
            if ((mask & 1 << slotIdx) != 0)
            {
                m_armors.at(slotIdx) = armor;
            }
        }
    }

    void SosOutfit::SwapArmor(Armor *armor)
    {
        PapyrusEvent::GetInstance().CallSwapArmor(m_name, armor);
        AddArmor(armor);
    }

    void SosOutfit::SosRemoveArmor(const Armor *armor)
    {
        PapyrusEvent::GetInstance().CallRemoveArmor(m_name, armor);
        RemoveArmor(armor);
    }

    void SosOutfit::RemoveArmor(const Armor *armor)
    {
        auto mask = static_cast<uint32_t>(armor->GetSlotMask());
        m_slotMask.reset(armor->GetSlotMask());
        for (uint32_t slotIdx = 0; slotIdx < SLOT_COUNT; ++slotIdx)
        {
            if ((mask & 1 << slotIdx) != 0)
            {
                m_armors.at(slotIdx) = nullptr;
            }
        }
    }

    constexpr int BODY_SLOT_MIN = 29;

    void SosOutfit::RenderProperties()
    {
        static std::array<char, 256> outfitNameBuf;
        ImGui::InputText("##OutfitRenameInput", outfitNameBuf.data(), outfitNameBuf.size());
        ImGui::SameLine();
        ImGui::BeginDisabled(outfitNameBuf.at(0) == '\0');
        if (ImGuiUtil::Button("$SkyOutSys_OContext_Rename"))
        {
            PapyrusEvent::GetInstance().CallRenameOutfit(m_name, outfitNameBuf.data());
            m_name.assign(outfitNameBuf.data());
            UpdateWindowTitle();
        }
        ImGui::EndDisabled();
    }

    void SosOutfit::RenderArmorList()
    {
        if (IsEmpty())
        {
            ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
            ImGuiUtil::Text("$SosGui_EmptyHint{$ARMOR}");
            ImGui::PopFontSize();
            return;
        }

        // render all in order but skip empty slot
        static int selectedIdx = -1;
        if (BeginTable(m_armorListTable))
        {
            TableHeadersRow(m_armorListTable);
            for (int slotIdx = 0; slotIdx < SLOT_COUNT; ++slotIdx)
            {
                const auto &armor = m_armors.at(slotIdx);
                if (armor == nullptr)
                {
                    continue;
                }

                ImGui::PushID(slotIdx);
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
                    m_armorRemoveConfirmPopup.Open(armor, [this](Armor *a_armor) {
                        SosRemoveArmor(a_armor);
                    });
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        if (auto *armor = m_armorRemoveConfirmPopup.GetData(); armor != nullptr)
        {
            const auto  templateS = Translation::TranslateIgnoreNested("$SkyOutSys_Confirm_RemoveArmor_Text{}");
            const auto *name      = armor->GetName();
            const auto  msg       = std::vformat(templateS, std::make_format_args(name));
            m_armorRemoveConfirmPopup.Render(msg);
        }
        else
        {
            m_armorRemoveConfirmPopup.Render("Invalid");
        }
    }

    void SosOutfit::RenderEditPanel()
    {
        auto selectedSlot = RenderArmorSlotFilter();

        ImGuiStyle &style     = ImGui::GetStyle();
        float       halfWidth = (ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2;

        ImGui::BeginChild("##RenderArmorCandidates", {halfWidth, 0.0F});
        RenderArmorCandidates(selectedSlot);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##RenderEditPanelPolicy", {halfWidth, 0.0F});
        RenderEditPanelPolicy();
        ImGui::EndChild();
    }

    auto SosOutfit::RenderArmorSlotFilter() -> Slot
    {
        static int selectedIdx = 0;

        if (ImGui::BeginCombo("##ArmorSlotFilter", ARMOR_SLOT_NAMES.at(selectedIdx)))
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

    void SosOutfit::RenderArmorCandidates(Slot selectedSlot)
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
        auto &outfitCandidates = SosUiData::GetInstance().GetArmorCandidatesCopy();
        ImGui::PushFontSize(HintFontSize());
        ImGuiUtil::Text(outfitCandidates.empty() ? "$SosGui_EmptyHint{$ARMOR}" : "");
        ImGui::PopFontSize();

        if (outfitCandidates.empty() || !BeginTable(m_armorCandidatesTable))
        {
            return;
        }

        m_armorCandidatesTable.rows = pageSize;
        int        count            = 0;
        int        startIdx         = currentPage * pageSize;
        static int selectedIdx      = -1;
        auto       begin            = outfitCandidates.begin() + startIdx;
        TableHeadersRow(m_armorCandidatesTable);
        for (auto &armorIter = begin; count < pageSize && armorIter != outfitCandidates.end(); ++count)
        {
            ImGui::PushID(count);
            ImGui::TableNextRow();
            {
                auto *armor = *armorIter;
                ImGui::TableNextColumn();
                bool isSelected = selectedIdx == count;
                if (ImGui::Selectable(armor->GetName(), isSelected, SELECTABLE_SPAN_FLAGS))
                {
                    selectedIdx = isSelected ? -1 : count;
                }

                ImGui::TableNextColumn();
                ImGui::Text("%d", static_cast<uint32_t>(armor->GetSlotMask()));

                ImGui::TableNextColumn();
                if (ImGuiUtil::Button("$Add"))
                {
                    if (IsConflictWith(armor))
                    {
                        m_armorConflictConfirmPopup.Open(armor, [this, &outfitCandidates](Armor *a_armor) {
                            SwapArmor(a_armor);
                            std::erase(outfitCandidates, a_armor);
                        });
                    }
                    else
                    {
                        PapyrusEvent::GetInstance().CallAddToOutfit(m_name, armor);
                        AddArmor(armor);
                        armorIter = outfitCandidates.erase(armorIter);
                        ImGui::PopID();
                        continue;
                    }
                }
            }
            ++armorIter;
            ImGui::PopID();
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
        auto msg = Translation::Translate("$SosGui_Confirm_ArmorConflict");
        m_armorConflictConfirmPopup.Render(msg);
    }

    void SosOutfit::RenderEditPanelPolicy()
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
            ImGui::PushID(idx);

            if (ImGui::RadioButton(policy.c_str(), idx == m_armorAddPolicy))
            {
                fSelected        = m_armorAddPolicy != idx;
                m_armorAddPolicy = idx;
            }
            ImGui::PopID();
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
            RenderOutfitAddPolicyById(m_fFilterPlayable);
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

    void SosOutfit::RenderOutfitAddPolicyById(const bool &fFilterPlayable) const
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
                PapyrusEvent::GetInstance().CallAddToOutfit(m_name, armor);
            }
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }

    auto SosOutfit::ArmorRemovePopup(const Armor *armor, bool &closed) const -> bool
    {
        if (armor == nullptr)
        {
            return false;
        }

        ImGuiStyle &style    = ImGui::GetStyle();
        float       width    = (ImGui::GetContentRegionAvail().x - 3 * style.ItemSpacing.x) / 4;
        bool        toRemove = false;
        if (ImGuiUtil::BeginPopupModal("$SosGui_Confirm_ArmorDelete"))
        {
            static auto removeHintTemplate =
                Translation::TranslateIgnoreNested("$SkyOutSys_Confirm_RemoveArmor_Text{}");

            const auto  name       = armor->GetName();
            std::string removeHint = std::vformat(removeHintTemplate, std::make_format_args(name));
            ImGui::TextWrapped("%s", removeHint.c_str());
            ImGui::SetCursorPosX(width / 2.0F);
            if (ImGuiUtil::Button("SkyOutSys_Confirm_RemoveArmor_Yes", {width, 0.0F}))
            {
                toRemove = true;
                closed   = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SetCursorPosX(width * 2.5F);
            if (!ImGuiUtil::Button("SkyOutSys_Confirm_RemoveArmor_No", {width, 0.0F}))
            {
                closed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (toRemove)
        {
            PapyrusEvent::GetInstance().CallRemoveArmor(m_name, armor);
        }
        return toRemove;
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