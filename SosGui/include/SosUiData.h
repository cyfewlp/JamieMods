//
// Created by jamie on 2025/4/3.
//

#ifndef SOSUIDATA_H
#define SOSUIDATA_H

#pragma once

#include "GuiContext.h"
#include "SosDataType.h"
#include "common/config.h"
#include "common/log.h"
#include "coroutine.h"
#include "data/PagedArmorList.h"
#include "data/SosUiOutfit.h"

#include <RE/A/Actor.h>
#include <boost/optional.hpp>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <cstdint>
#include <list>
#include <mutex>
#include <queue>
#include <string>
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
        using OutfitState   = std::pair<StateType, std::string>;
        using BodySlotArmor = std::pair<BodySlot, Armor *>;
        using OutfitList    = std::unordered_map<SosUiOutfit::OutfitId, SosUiOutfit>;
        using OutfitPair    = std::pair<SosUiOutfit::OutfitId, SosUiOutfit *>;

        static constexpr uint8_t            DEFAULT_PAGE_SIZE = 20;
        static inline SosUiOutfit::OutfitId g_NextOutfitId    = 1;

    private:
        GuiContext                                   m_context;
        std::vector<RE::Actor *>                     m_actors;
        std::vector<RE::Actor *>                     m_NearActors;
        bool                                         m_enabled           = false;
        bool                                         m_fQuickSlotEnabled = false;
        PagedArmorList                               m_armorCandidates{DEFAULT_PAGE_SIZE};
        std::unordered_map<RE::Actor *, std::string> m_actorActiveOutfitMap;
        OutfitList                                   m_outfitList;
        std::list<std::string>                       m_errorMessages;

        std::unordered_map<RE::Actor *, bool>                                       m_autoSwitchEnabled;
        std::unordered_map<RE::Actor *, std::unordered_map<StateType, std::string>> m_actorOutfitStates;

        std::queue<CoroutineTask>           uiTasks;
        std::queue<std::coroutine_handle<>> m_resumeQueue;
        std::mutex                          m_mutex;

    public:
        ////////////////////////////////////////////////////////////////////////////
        // Context
        ////////////////////////////////////////////////////////////////////////////

        [[nodiscard]] constexpr auto GetContext() -> GuiContext & { return m_context; }

        ////////////////////////////////////////////////////////////////////////////
        // UI Tasks
        ////////////////////////////////////////////////////////////////////////////

        struct awaitable
        {
            SosUiData *self;

            bool await_ready() const noexcept { return false; }

            auto await_suspend(std::coroutine_handle<> a_handle) const noexcept
            {
                std::lock_guard lock(self->m_mutex);
                self->m_resumeQueue.push(a_handle);
            }

            void await_resume() {}
        };

        awaitable await_execute_on_ui() { return awaitable{this}; }

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
                    if (coroutineHandle) { coroutineHandle(); }
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

        [[nodiscard]] auto GetActors() const -> const std::vector<RE::Actor *> & { return m_actors; }

        [[nodiscard]] auto GetActors() -> std::vector<RE::Actor *> & { return m_actors; }

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
            if (actor != nullptr && std::ranges::find(m_actors, actor) == m_actors.end()) { m_actors.push_back(actor); }
        }

        void RemoveActor(RE::Actor *actor) { std::erase(m_actors, actor); }

        [[nodiscard]] constexpr auto IsQuickSlotEnabled() const -> bool { return m_fQuickSlotEnabled; }

        void SetQuickSlotEnabled(const bool fQuickSlotEnabled) { m_fQuickSlotEnabled = fQuickSlotEnabled; }

        [[nodiscard]] auto GetNearActors() const -> const std::vector<RE::Actor *> & { return m_NearActors; }

        void SetNearActors(std::vector<RE::Actor *> &nearActors)
        {
            m_NearActors.clear();
            m_NearActors = std::move(nearActors);
        }

        [[nodiscard]] auto IsEnabled() const -> bool { return m_enabled; }

        void SetEnabled(const bool enabled) { m_enabled = enabled; }

        [[nodiscard]] auto IsAutoSwitchEnabled(RE::Actor *actor) const -> bool
        {
            return m_autoSwitchEnabled.contains(actor) ? m_autoSwitchEnabled.at(actor) : false;
        }

        void SetAutoSwitchEnabled(RE::Actor *actor, const bool autoSwitchEnabled)
        {
            if (actor != nullptr)
            {
                m_autoSwitchEnabled[actor] = autoSwitchEnabled;
                m_actorOutfitStates.clear();
            }
        }

        [[nodiscard]] auto GetActorOutfitStates() const
            -> const std::unordered_map<RE::Actor *, std::unordered_map<StateType, std::string>> &
        {
            return m_actorOutfitStates;
        }

        void PutActorOutfitState(RE::Actor *actor, OutfitState &&state)
        {
            m_actorOutfitStates.emplace(actor, std::unordered_map<StateType, std::string>());
            auto &stateMap = m_actorOutfitStates.at(actor);
            stateMap.insert_or_assign(state.first, state.second);
        }

        auto GetActorOutfitByState(RE::Actor *actor, const StateType &state) const -> const std::string &
        {
            static std::string empty = "";
            if (!m_actorOutfitStates.contains(actor)) { return empty; }
            auto &stateMap = m_actorOutfitStates.at(actor);
            return stateMap.contains(state) ? stateMap.at(state) : empty;
        }

        ////////////////////////////////////////////////////////////////////////////
        // Candidate Armor
        ////////////////////////////////////////////////////////////////////////////

        [[nodiscard]] constexpr auto GetArmorCandidates() -> PagedArmorList & { return m_armorCandidates; }

        void MarkArmorIsUsed(Armor *armor) { m_armorCandidates.Insert(armor, true); }

        void MarkArmorIsUnused(Armor *armor) { m_armorCandidates.Insert(armor, false); }

        void DeleteCandidateArmor(Armor *armor) { m_armorCandidates.Remove(armor); }

        void SetArmorCandidates(const std::vector<Armor *> &armorCandidates)
        {
            m_armorCandidates.Clear();
            for (const auto &armor : armorCandidates)
            {
                MarkArmorIsUsed(armor);
            }
        }

        auto GetCandidateArmorCount() const -> uint32_t { return m_armorCandidates.Size(); }

        auto GetAllArmorCount() const -> uint32_t { return m_armorCandidates.GetAllArmorCount(); }

        ////////////////////////////////////////////////////////////////////////////
        // SosUiOutfit
        ////////////////////////////////////////////////////////////////////////////

        template <typename String>
        constexpr void AddOutfit(String &&outfitName)
        {
            m_outfitList.emplace(std::piecewise_construct, std::forward_as_tuple(g_NextOutfitId),
                                 std::forward_as_tuple(g_NextOutfitId, std::move(outfitName)));
            ++g_NextOutfitId;
        }

        void AddOutfits(auto &&container)
        {
            for (const auto &outfit : container)
            {
                AddOutfit(outfit);
            }
        }

        auto GetOutfit(const SosUiOutfit::OutfitId id) -> boost::optional<SosUiOutfit &>
        {
            boost::optional<SosUiOutfit &> opt;
            if (m_outfitList.contains(id)) { opt = m_outfitList.at(id); }
            return opt;
        }

        void RenameOutfit(const SosUiOutfit::OutfitId id, const std::string &&newName)
        {
            if (auto opt = GetOutfit(id); opt) { opt.value().SetName(newName); }
        }

        void AddArmor(const SosUiOutfit::OutfitId id, Armor *armor)
        {
            if (auto opt = GetOutfit(id); opt) { opt.value().AddArmor(armor); }
        }

        void AddArmors(const SosUiOutfit::OutfitId id, const std::vector<Armor *> &armors)
        {
            if (!m_outfitList.contains(id)) { return; }
            for (const auto &armor : armors)
            {
                AddArmor(id, armor);
            }
        }

        void DeleteOutfit(const SosUiOutfit::OutfitId id) { m_outfitList.erase(id); }

        void DeleteArmor(const SosUiOutfit::OutfitId id, const Armor *armor)
        {
            if (auto opt = GetOutfit(id); opt) { opt.value().RemoveArmor(armor); }
        }

        [[nodiscard]] constexpr auto GetOutfitList() -> OutfitList & { return m_outfitList; }

        bool HasOutfit(SosUiOutfit::OutfitId &id) { return m_outfitList.contains(id); }

        ////////////////////////////////////////////////////////////////////////////
        // Active outfit
        ////////////////////////////////////////////////////////////////////////////

        [[nodiscard]] auto GetActorActiveOutfitMap() const -> const std::unordered_map<RE::Actor *, std::string> &
        {
            return m_actorActiveOutfitMap;
        }

        void SetActorActiveOutfit(RE::Actor *actor, const std::string &outfitName)
        {
            m_actorActiveOutfitMap[actor] = outfitName;
        }

        auto HasActiveOutfit(RE::Actor *actor) const -> bool
        {
            return m_actorActiveOutfitMap.contains(actor) && !m_actorActiveOutfitMap.at(actor).empty();
        }

        auto IsActorActiveOutfit(RE::Actor *actor, const std::string &outfitName) const -> bool
        {
            return m_actorActiveOutfitMap.contains(actor) && m_actorActiveOutfitMap.at(actor) == outfitName;
        }

        void PushErrorMessage(std::string &&message)
        {
            while (m_errorMessages.size() >= MAX_ERROR_COUNT)
            {
                m_errorMessages.erase(m_errorMessages.begin());
            }
            m_errorMessages.emplace_back(std::move(message));
        }

        [[nodiscard]] auto GetErrorMessages() -> std::list<std::string> & { return m_errorMessages; }
    };
}

#endif // SOSUIDATA_H