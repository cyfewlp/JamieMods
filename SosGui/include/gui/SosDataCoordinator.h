#pragma once

#include "SosUiData.h"
#include "common/config.h"

#include <RE/A/Actor.h>
#include <coroutine>
#include <functional>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    struct CoroutinePromise
    {
        struct promise_type
        {
            static CoroutinePromise get_return_object()
            {
                return {};
            }

            static std::suspend_never initial_suspend()
            {
                return {};
            }

            static std::suspend_never final_suspend() noexcept
            {
                return {};
            }

            void return_void() const
            {
            }

            static void unhandled_exception()
            {
            }
        };
    };

    class SosDataCoordinator
    {
        SosUiData &m_uiData;
        using OnComplete = std::function<void()>;
        using Variable   = RE::BSScript::Variable;
        using Armor      = RE::TESObjectARMO;

    public:
        explicit SosDataCoordinator(SosUiData &uiData) : m_uiData(uiData)
        {
        }

        auto RequestEnable(bool isEnabled) const -> CoroutinePromise;
        auto RequestImportSettings() -> CoroutinePromise;
        auto RequestExportSettings() const -> CoroutinePromise;

        auto RequestCreateOutfit(std::string outfitName) const -> CoroutinePromise;
        auto RequestCreateOutfitFromWorn(std::string outfitName) const -> CoroutinePromise;
        auto RequestOutfitList() const -> CoroutinePromise;
        auto RequestRenameOutfit(SosUiData::OutfitPair pair, std::string newName) const -> CoroutinePromise;
        auto RequestDeleteOutfit(SosUiData::OutfitPair pair) const -> CoroutinePromise;
        auto RequestAddArmor(SosUiData::OutfitPair pair, Armor *armor) const -> CoroutinePromise;
        auto RequestDeleteArmor(SosUiData::OutfitPair pair, Armor *armor) const -> CoroutinePromise;
        auto RequestOutfitArmors(SosUiData::OutfitPair pair) const -> CoroutinePromise;

        auto RequestGetArmorsByCarried() const -> CoroutinePromise;
        auto RequestGetArmorsByWorn() const -> CoroutinePromise;

        auto RequestUpdateActorAutoSwitchState(RE::Actor *actor) const -> CoroutinePromise;
        auto RequestSetActorAutoSwitchState(RE::Actor *actor, bool enabled) const -> CoroutinePromise;
        auto RequestActorStateOutfit(RE::Actor *actor, StateType location) const -> CoroutinePromise;
        auto RequestSetActorStateOutfit(RE::Actor *actor, StateType location, std::string outfitName) const -> CoroutinePromise;

        auto RequestActorList(OnComplete onComplete = {}) const -> CoroutinePromise;
        auto RequestAddActor(RE::Actor *actor) const -> CoroutinePromise;
        auto RequestRemoveActor(RE::Actor *actor) const -> CoroutinePromise;
        auto RequestNearActorList() const -> CoroutinePromise;
        auto RequestActiveOutfit(RE::Actor *actor, std::string outfitName, OnComplete onComplete = {}) const -> CoroutinePromise;

        auto Refresh() -> CoroutinePromise;

        static auto HasQuickSlotSpell() -> bool;
    };
}