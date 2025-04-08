//
// Created by jamie on 2025/4/6.
//

#ifndef SOSOUTFIT_H
#define SOSOUTFIT_H

#pragma once

#include "ImGuiUtil.h"
#include "SosDataType.h"
#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
    class SosOutfit
    {
        using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor = RE::TESObjectARMO;

        static constexpr int MAX_FILTER_ARMOR_NAME = 256;
        static constexpr int SLOT_COUNT            = 32;
        static constexpr int SOS_SLOT_OFFSET       = 30;

        std::string                     m_name;
        std::string                     m_windowTitle;
        SKSE::stl::enumeration<Slot>    m_slotMask = Slot::kNone;
        std::array<Armor *, SLOT_COUNT> m_armors;
        // ui variables
        ImGuiUtil::ImTable<3>                   m_armorListTable;
        ImGuiUtil::ImTable<3>                   m_armorCandidatesTable;
        int                                     m_armorAddPolicy  = 0;
        bool                                    m_fFilterPlayable = false;
        std::array<char, MAX_FILTER_ARMOR_NAME> m_filterStringBuf;

        bool m_fShowOutfitWindow = false;

        class ConfirmPopup
        {
            std::string name;
            ImGuiID     id;
            Armor      *data      = nullptr;
            bool        isOpen    = false;
            bool        isConfirm = false;

            std::function<void(Armor *)> onConfirm = nullptr;

        public:
            explicit ConfirmPopup(const std::string_view &nameKey) : id(0)
            {
                name = Translation::Translate(nameKey.data());
            }

            void Open(Armor *data, const std::function<void(Armor *)> &callback);
            void Close();
            void Render(const std::string_view &message);

            constexpr auto GetData() const -> Armor *
            {
                return data;
            }
        };

        ConfirmPopup m_armorConflictConfirmPopup;
        ConfirmPopup m_armorRemoveConfirmPopup;

    public:
        void UpdateWindowTitle();

        explicit SosOutfit(const std::string &name)
            : m_armors(), m_filterStringBuf(), //
              m_armorConflictConfirmPopup("$SosGui_Confirm_ArmorConflict"),
              m_armorRemoveConfirmPopup("$SosGui_Confirm_ArmorDelete")
        {
            m_name                = name;
            m_armorListTable.name = "##OutfitArmors";
            m_armorListTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
            m_armorListTable.headersRow = {Translation::Translate("$SosGui_TableHeader_Slot"),
                                           Translation::Translate("$ARMOR"), Translation::Translate("$Delete")};

            m_armorCandidatesTable.name = "##ArmorCandidates";
            m_armorCandidatesTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
            m_armorCandidatesTable.headersRow = {
                Translation::Translate("$ARMOR"),
                Translation::Translate("$SosGui_TableHeader_Slot"),
                Translation::Translate("$Add"),
            };
            UpdateWindowTitle();
        }

        ~SosOutfit() = default;

        auto Render() -> bool;

        void AddArmor(Armor *armor);
        void RemoveArmor(const Armor *armor);

        void SwapArmor(Armor *armor);
        void SosRemoveArmor(const Armor *armor);

        constexpr void ShowOutfitWindow(bool show)
        {
            m_fShowOutfitWindow = show;
            if (show)
            {
                auto policy = static_cast<OutfitAddPolicy>(m_armorAddPolicy);
                UpdateArmorCandidates(m_filterStringBuf.data(), m_fFilterPlayable, policy);
            }
        }

        constexpr auto IsConflictWith(const Armor *armor) const -> bool
        {
            return m_slotMask.any(armor->GetSlotMask());
        }

        constexpr auto IsEmpty() const -> bool
        {
            return m_slotMask.underlying() == 0;
        }

        [[nodiscard]] constexpr auto GetName() const -> const std::string &
        {
            return m_name;
        }

    private:
        void        RenderProperties();
        void        RenderArmorList();
        void        RenderEditPanel();
        static auto RenderArmorSlotFilter() -> Slot;
        void        RenderArmorCandidates(Slot selectedSlot);
        void        RenderOutfitAddPolicyById(const bool &fFilterPlayable) const;
        void        RenderEditPanelPolicy();
        auto        ArmorRemovePopup(const Armor *armor, bool &closed) const -> bool;
        static void UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                          OutfitAddPolicy policy);
        static void UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable);
        static void UpdateArmorCandidatesBySlot(Slot slot);
        static void FilterArmorCandidates(const std::string_view &filterString, std::vector<Armor *> &armorCandidates);
        static auto IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool;
    };
}

#endif // SOSOUTFIT_H
