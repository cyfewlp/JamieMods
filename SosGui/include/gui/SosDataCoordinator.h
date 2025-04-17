#pragma once

#include "SosUiData.h"
#include "common/config.h"
#include "coroutine.h"

#include <RE/A/Actor.h>
#include <functional>
#include <string>

namespace LIBC_NAMESPACE_DECL
{

    class SosDataCoordinator
    {
        SosUiData &m_uiData;
        using OnComplete = std::function<void()>;
        using Variable   = RE::BSScript::Variable;
        using Armor      = RE::TESObjectARMO;

    public:
        explicit SosDataCoordinator(SosUiData &uiData) : m_uiData(uiData) {}

        auto RequestEnable(bool isEnabled) const -> CoroutineTask;
        auto QueryIsEnable() const -> CoroutinePromise;
        auto RequestImportSettings() -> CoroutineTask;
        auto RequestExportSettings() const -> CoroutineTask;

        auto RequestCreateOutfit(std::string outfitName) const -> CoroutineTask;
        auto RequestCreateOutfitFromWorn(std::string outfitName) const -> CoroutineTask;
        auto RequestOutfitList() const -> CoroutinePromise;
        auto RequestRenameOutfit(SosUiData::OutfitPair pair, std::string newName) const -> CoroutineTask;
        auto RequestDeleteOutfit(SosUiData::OutfitPair pair) const -> CoroutineTask;
        auto RequestAddArmor(SosUiData::OutfitPair pair, Armor *armor) const -> CoroutineTask;
        auto RequestDeleteConflictArmorsWith(SosUiData::OutfitPair pair, Armor *armor) const -> CoroutineTask;
        auto RequestDeleteArmor(SosUiData::OutfitPair pair, Armor *armor) const -> CoroutineTask;
        auto RequestOutfitArmors(SosUiData::OutfitPair pair) const -> CoroutineTask;
        auto RequestSetOutfitSlotPolicy(SosUiData::OutfitPair pair, uint32_t slotPos, SlotPolicy policy) const -> CoroutineTask;
        auto RequestOutfitSlotPolicy(SosUiData::OutfitPair pair) const -> CoroutineTask;

        auto RequestGetArmorsByCarried() const -> CoroutineTask;
        auto RequestGetArmorsByWorn() const -> CoroutineTask;

        auto RequestUpdateActorAutoSwitchState(RE::Actor *actor) const -> CoroutinePromise;
        auto RequestSetActorAutoSwitchState(RE::Actor *actor, bool enabled) const -> CoroutineTask;
        auto RequestActorStateOutfit(RE::Actor *actor, StateType location) const -> CoroutineTask;
        auto RequestSetActorStateOutfit(RE::Actor *actor, StateType location, std::string outfitName) const
            -> CoroutineTask;

        auto RequestActorList() const -> CoroutinePromise;
        auto RequestAddActor(RE::Actor *actor) const -> CoroutineTask;
        auto RequestRemoveActor(RE::Actor *actor) const -> CoroutineTask;
        auto RequestNearActorList() const -> CoroutineTask;
        auto RequestActiveOutfit(RE::Actor *actor, std::string outfitName) const -> CoroutineTask;

        auto Refresh() const -> CoroutineTask;

        static auto HasQuickSlotSpell() -> bool;
    };
}