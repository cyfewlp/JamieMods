#pragma once

#include "SosDataType.h"
#include "common/config.h"
#include "coroutine.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/id.h"

#include <RE/A/Actor.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/V/Variable.h>
#include <cstdint>
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

        auto CreateOutfit(std::string outfitName) const -> CoroutineTask;

        auto CreateOutfitFromWorn(std::string outfitName) const -> CoroutineTask;

        auto GetOutfitList() const -> CoroutinePromise;

        auto RequestFavoriteOutfits() const -> CoroutinePromise;

        auto SetActorOutfit(RE::Actor *actor, OutfitId id, std::string outfitName) const -> CoroutineTask;

        auto GetActorOutfit(RE::Actor *actor) const -> CoroutineTask;

        auto RenameOutfit(OutfitId id, std::string outfitName, std::string newName) const -> CoroutineTask;

        auto DeleteOutfit(OutfitId id, std::string outfitName) const -> CoroutineTask;

        auto AddArmor(OutfitId id, std::string outfitName, Armor *armor) const -> CoroutineTask;

        auto DeleteConflictArmors(std::string outfitName, Armor *armor) const -> CoroutineTask;

        auto DeleteArmor(OutfitId id, std::string outfitName, Armor *armor) const -> CoroutineTask;

        auto GetOutfitArmors(OutfitId id, std::string outfitName) const -> CoroutineTask;

        auto SetSlotPolicy(OutfitId id, std::string outfitName, uint32_t slotPos, SlotPolicy policy) const
            -> CoroutineTask;

        auto GetSlotPolicy(OutfitId id, std::string outfitName) const -> CoroutineTask;

        auto GetActorStateOutfit(RE::Actor *actor, StateType location) const -> CoroutineTask;

        auto SetActorStateOutfit(RE::Actor *actor, StateType location, std::string outfitName) const -> CoroutineTask;
    };
}