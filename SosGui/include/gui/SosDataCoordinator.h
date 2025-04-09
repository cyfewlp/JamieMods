#pragma once

#include "SosUiData.h"
#include "common/config.h"

#include <RE/A/Actor.h>
#include <coroutine>
#include <functional>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    struct CoroutineTask
    {
        struct promise_type
        {
            static CoroutineTask get_return_object()
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

    public:
        explicit SosDataCoordinator(SosUiData &uiData) : m_uiData(uiData)
        {
        }

        auto RequestEnable(bool isEnabled) -> CoroutineTask;
        auto RequestImportSettings() -> CoroutineTask;
        auto RequestExportSettings() -> CoroutineTask;

        auto RequestCreateOutfit(std::string outfitName) -> CoroutineTask;
        auto RequestCreateOutfitFromWorn(std::string outfitName) -> CoroutineTask;
        auto RequestOutfitList(OnComplete onComplete = {}) -> CoroutineTask;
        auto RequestUpdateActorAutoSwitchState(RE::Actor *actor, OnComplete onComplete = {}) -> CoroutineTask;
        auto RequestDeleteOutfit(std::string outfitName, OnComplete onComplete = {}) -> CoroutineTask;
        auto RequestOutfitArmors(std::string outfitName) -> CoroutineTask;

        auto RequestActorList(OnComplete onComplete = {}) -> CoroutineTask;
        auto RequestActiveOutfit(RE::Actor *actor, std::string outfitName, OnComplete onComplete = {}) -> CoroutineTask;

        auto Refresh() -> CoroutineTask;

        auto HasQuickSlotSpell() -> bool;
    };
}