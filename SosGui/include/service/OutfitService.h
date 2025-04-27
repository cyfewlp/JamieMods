#pragma once

#include "SosDataType.h"
#include "common/config.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/id.h"
#include "task.h"

#include <RE/T/TESObjectARMO.h>
#include <RE/V/Variable.h>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    class OutfitService
    {
        using Armor    = RE::TESObjectARMO;
        using Variable = RE::BSScript::Variable;
        SosUiData  &m_uiData;
        OutfitList &m_outfitList;

    public:
        OutfitService(SosUiData &uiData) : m_uiData(uiData), m_outfitList(m_uiData.GetOutfitList()) {}

        auto CreateOutfit(std::string outfitName) const -> Task;

        auto CreateOutfitFromWorn(std::string outfitName) const -> Task;

        auto GetOutfitList() const -> Task;

        auto GetAllFavoriteOutfits() const -> Task;

        auto SetOutfitIsFavorite(OutfitId id, std::string outfitName, bool isFavorite) const -> Task;

        auto SetActorOutfit(RE::Actor *actor, OutfitId id, std::string outfitName) const -> Task;

        auto GetActorOutfit(RE::Actor *actor) const -> Task;

        auto RenameOutfit(OutfitId id, std::string outfitName, std::string newName) const -> Task;

        auto DeleteOutfit(OutfitId id, std::string outfitName) const -> Task;

        auto AddArmor(OutfitId id, std::string outfitName, Armor *armor) const -> Task;

        static auto DeleteConflictArmors(std::string outfitName, Armor *armor) -> Task;

        auto DeleteArmor(OutfitId id, std::string outfitName, Armor *armor) const -> Task;

        auto GetOutfitArmors(OutfitId id, std::string outfitName) const -> Task;

        auto SetSlotPolicy(OutfitId id, std::string outfitName, uint32_t slotPos, SlotPolicy policy) const -> Task;

        auto GetSlotPolicy(OutfitId id, std::string outfitName) const -> Task;

        auto GetActorStateOutfit(RE::Actor *actor, uint32_t policyId) const -> Task;

        auto GetActorAllStateOutfit(RE::Actor *actor) const -> Task;

        auto SetActorStateOutfit(RE::Actor *actor, uint32_t policyId, OutfitId outfitId) const -> Task;
    };
}