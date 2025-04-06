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

        std::string           m_name;
        std::vector<Armor *>  m_armors;
        ImGuiUtil::ImTable<2> m_armorListTable;
        ImGuiUtil::ImTable<3> m_armorCandidatesTable;

    public:
        SosOutfit(const std::string &name) : m_name(name)
        {
            m_armorListTable.name = "##OutfitArmors";
            m_armorListTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
            m_armorListTable.flags |= ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame;
            m_armorListTable.headersRow = {ImGuiUtil::Translate("$SosGui_TableHeader_Slot"),
                                           ImGuiUtil::Translate("$ARMOR")};

            m_armorCandidatesTable.name = "##ArmorCandidates";
            m_armorCandidatesTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
            m_armorCandidatesTable.flags |= ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame;
            m_armorCandidatesTable.headersRow = {
                ImGuiUtil::Translate("$ARMOR"),
                ImGuiUtil::Translate("$SosGui_TableHeader_Slot"),
                ImGuiUtil::Translate("$Add"),
            };
        }

        ~SosOutfit() = default;

        void Render();

        void AddArmor(Armor *armor)
        {
            m_armors.push_back(armor);
        }

    private:
        void        RenderArmorList();
        void        RenderEditPanel();
        static auto RenderArmorSlotFilter() -> Slot;
        void        RenderAddPolicyByCandidates();
        static void RenderOutfitAddPolicyById(const std::string &outfitName, const bool &fFilterPlayable);
        void        RenderEditPanelPolicy() const;
        bool        TryAddArmor(const std::string &outfitName, const Armor *armor) const;
        static void UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                          OutfitAddPolicy policy);
        static void UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable);
        static void UpdateArmorCandidatesBySlot(Slot slot);
        static void FilterArmorCandidates(const std::string_view &filterString, std::vector<Armor *> &armorCandidates);
        static auto IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool;
    };
}

#endif // SOSOUTFIT_H
