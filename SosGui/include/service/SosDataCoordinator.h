#pragma once

#include "common/config.h"
#include "coroutine.h"
#include "data/SosUiData.h"
#include "data/id.h"
#include "service/OutfitService.h"

#include <RE/A/Actor.h>
#include <functional>
#include <string>

namespace LIBC_NAMESPACE_DECL
{

    class SosDataCoordinator
    {
        SosUiData     &m_uiData;
        OutfitService &m_outfitService;
        using OnComplete = std::function<void()>;
        using Variable   = RE::BSScript::Variable;
        using Armor      = RE::TESObjectARMO;

    public:
        explicit SosDataCoordinator(SosUiData &uiData, OutfitService &outfitService)
            : m_uiData(uiData), m_outfitService(outfitService)
        {
        }

        auto RequestEnable(bool isEnabled) const -> CoroutineTask;
        auto QueryIsEnable() const -> CoroutinePromise;
        auto RequestImportSettings() -> CoroutineTask;
        auto RequestExportSettings() const -> CoroutineTask;
        auto RequestFavoriteOutfits() const -> CoroutinePromise;
        auto RequestGetArmorsByCarried() const -> CoroutineTask;
        auto RequestGetArmorsByWorn() const -> CoroutineTask;
        auto RequestUpdateActorAutoSwitchState(RE::Actor *actor) const -> CoroutinePromise;
        auto RequestSetActorAutoSwitchState(RE::Actor *actor, bool enabled) const -> CoroutineTask;
        auto RequestActorList() const -> CoroutinePromise;
        auto RequestAddActor(RE::Actor *actor) const -> CoroutineTask;
        auto RequestRemoveActor(RE::Actor *actor) const -> CoroutineTask;
        auto RequestNearActorList() const -> CoroutineTask;
        auto Refresh() const -> CoroutineTask;

        static auto HasQuickSlotSpell() -> bool;
    };
}