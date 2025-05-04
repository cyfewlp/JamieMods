//
// Created by jamie on 2025/4/3.
//

#ifndef SOSUIDATA_H
#define SOSUIDATA_H

#pragma once

#include "common/config.h"
#include "common/log.h"
#include "data/ActorOutfitMap.h"
#include "data/AutoSwitchPolicyView.h"
#include "data/OutfitList.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/ErrorNotifier.h"

#include <RE/A/Actor.h>
#include <RE/T/TESObjectARMO.h>
#include <coroutine>
#include <exception>
#include <list>
#include <mutex>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
class SosUiData
{
public:
    using Armor         = RE::TESObjectARMO;
    using BodySlot      = int32_t;
    using BodySlotArmor = std::pair<BodySlot, Armor *>;
    using OutfitPair    = std::pair<OutfitId, const SosUiOutfit *>;

    static constexpr uint8_t DEFAULT_PAGE_SIZE = 20;
    static inline OutfitId   g_NextOutfitId    = 1;

private:
    std::vector<RE::Actor *> m_actors;
    std::vector<RE::Actor *> m_NearActors;
    bool                     m_enabled           = false;
    bool                     m_fQuickSlotEnabled = false;
    ActorOutfitMap           m_actorOutfitMap;
    OutfitList               m_outfitList{};
    AutoSwitchPolicyView     m_autoSwitchPolicyView;
    std::list<std::string>   m_errorMessages;
    ErrorNotifier            m_errorNotifier;

    std::unordered_map<RE::FormID, bool> m_autoSwitchEnabled;
    std::queue<std::coroutine_handle<>>  m_resumeQueue;
    std::mutex                           m_mutex;

public:
    ////////////////////////////////////////////////////////////////////////////
    // UI Tasks
    ////////////////////////////////////////////////////////////////////////////

    struct awaitable
    {
        SosUiData *self;

        bool await_ready() const noexcept
        {
            return false;
        }

        auto await_suspend(std::coroutine_handle<> a_handle) const noexcept
        {
            std::lock_guard lock(self->m_mutex);
            self->m_resumeQueue.push(a_handle);
        }

        void await_resume() {}
    };

    awaitable await_execute_on_ui()
    {
        return awaitable{this};
    }

    void ExecuteUiTasks()
    {
        std::queue<std::coroutine_handle<>> localQueue;
        {
            std::lock_guard lock(m_mutex);
            localQueue.swap(m_resumeQueue);
        }
        while (!localQueue.empty())
        {
            auto &coroutineHandle = localQueue.front();
            try
            {
                if (coroutineHandle)
                {
                    coroutineHandle();
                }
            }
            catch (const std::exception &ex)
            {
                log_error("Coroutine task occurs exception: ", ex.what());
                LogStacktrace();
            }
            localQueue.pop();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Actors
    ////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] auto GetActors() const -> const std::vector<RE::Actor *> &
    {
        return m_actors;
    }

    [[nodiscard]] auto GetActors() -> std::vector<RE::Actor *> &
    {
        return m_actors;
    }

    void SetActors(const std::vector<RE::Actor *> &actors)
    {
        m_actors.clear();
        for (const auto &actor : actors)
        {
            m_actors.push_back(actor);
        }
    }

    void AddActor(RE::Actor *actor)
    {
        if (actor != nullptr && std::ranges::find(m_actors, actor) == m_actors.end())
        {
            m_actors.push_back(actor);
        }
    }

    void RemoveActor(RE::Actor *actor)
    {
        std::erase(m_actors, actor);
    }

    [[nodiscard]] constexpr auto IsQuickSlotEnabled() const -> bool
    {
        return m_fQuickSlotEnabled;
    }

    void SetQuickSlotEnabled(const bool fQuickSlotEnabled)
    {
        m_fQuickSlotEnabled = fQuickSlotEnabled;
    }

    [[nodiscard]] auto GetNearActors() const -> const std::vector<RE::Actor *> &
    {
        return m_NearActors;
    }

    void SetNearActors(std::vector<RE::Actor *> &nearActors)
    {
        m_NearActors.clear();
        m_NearActors = std::move(nearActors);
    }

    [[nodiscard]] auto IsEnabled() const -> bool
    {
        return m_enabled;
    }

    void SetEnabled(const bool enabled)
    {
        m_enabled = enabled;
    }

    [[nodiscard]] auto IsAutoSwitchEnabled(const RE::FormID actorId) const -> bool
    {
        return m_autoSwitchEnabled.contains(actorId) ? m_autoSwitchEnabled.at(actorId) : false;
    }

    void SetAutoSwitchEnabled(const RE::FormID actorId, const bool autoSwitchEnabled)
    {
        m_autoSwitchEnabled[actorId] = autoSwitchEnabled;
    }

    auto GetAutoSwitchPolicyView() const -> const AutoSwitchPolicyView &
    {
        return m_autoSwitchPolicyView;
    }

    auto GetAutoSwitchPolicyView() -> AutoSwitchPolicyView &
    {
        return m_autoSwitchPolicyView;
    }

    ////////////////////////////////////////////////////////////////////////////
    // SosUiOutfit
    ////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] auto GetOutfitList() const -> const OutfitList &
    {
        return m_outfitList;
    }

    [[nodiscard]] auto GetOutfitList() -> OutfitList &
    {
        return m_outfitList;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Active outfit
    ////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] auto GetActorOutfitMap() const -> const ActorOutfitMap &
    {
        return m_actorOutfitMap;
    }

    [[nodiscard]] auto GetActorOutfitMap() -> ActorOutfitMap &
    {
        return m_actorOutfitMap;
    }

    ////////////////////////////////////////////////////////////////////////////
    // UI Error Messages
    ////////////////////////////////////////////////////////////////////////////

    void PushErrorMessage(std::string &&message)
    {
        m_errorNotifier.addError(message);
    }

    [[nodiscard]] auto GetErrorNotifier() -> ErrorNotifier &
    {
        return m_errorNotifier;
    }
};
}
#endif // SOSUIDATA_H