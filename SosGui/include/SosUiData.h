//
// Created by jamie on 2025/4/3.
//

#ifndef SOSUIDATA_H
#define SOSUIDATA_H

#pragma once

#include "SosDataType.h"
#include "common/config.h"
#include "data/PagedArmorList.h"
#include "data/SosUiOutfit.h"

#include <RE/A/Actor.h>
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{

    class SosUiData
    {
    public:
        using Armor               = RE::TESObjectARMO;
        using BodySlot            = int32_t;
        using OutfitState         = std::pair<StateType, std::string>;
        using BodySlotArmor       = std::pair<BodySlot, Armor *>;
        using OutfitList          = std::list<SosUiOutfit>;
        using OutfitConstIterator = std::list<SosUiOutfit>::const_iterator;
        using OutfitIterator      = std::list<SosUiOutfit>::iterator;

        static constexpr uint8_t DEFAULT_PAGE_SIZE = 20;

    private:
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

        std::queue<std::function<void(SosUiData &uiData)>> uiTasks;
        std::mutex                                         m_mutex;

    public:
        void PushTask(std::function<void(SosUiData &uiData)> &&task)
        {
            std::lock_guard lock(m_mutex);
            uiTasks.push(std::move(task));
        }

        void ExecuteUiTasks()
        {
            std::queue<std::function<void(SosUiData & uiData)>> localQueue;
            {
                std::lock_guard lock(m_mutex);
                localQueue.swap(uiTasks);
            }
            while (!localQueue.empty())
            {
                localQueue.front()(*this);
                localQueue.pop();
            }
        }

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
            if (!m_actorOutfitStates.contains(actor))
            {
                return empty;
            }
            auto &stateMap = m_actorOutfitStates.at(actor);
            return stateMap.contains(state) ? stateMap.at(state) : empty;
        }

        ////////////////////////////////////////////////////////////////////////////
        // Candidate Armor
        ////////////////////////////////////////////////////////////////////////////

        [[nodiscard]] constexpr auto GetArmorCandidates() -> PagedArmorList &
        {
            return m_armorCandidates;
        }

        void MarkArmorIsUsed(Armor *armor)
        {
            m_armorCandidates.Insert(armor, true);
        }

        void MarkArmorIsUnused(Armor *armor)
        {
            m_armorCandidates.Insert(armor, false);
        }

        void DeleteCandidateArmor(Armor *armor)
        {
            m_armorCandidates.Remove(armor);
        }

        void SetArmorCandidates(const std::vector<Armor *> &armorCandidates)
        {
            m_armorCandidates.Clear();
            for (const auto &armor : armorCandidates)
            {
                m_armorCandidates.Insert(armor);
            }
        }

        ////////////////////////////////////////////////////////////////////////////
        // SosUiOutfit
        ////////////////////////////////////////////////////////////////////////////

        void AddOutfit(const std::string &&outfitName)
        {
            m_outfitList.push_back(SosUiOutfit(outfitName));
        }

        void AddOutfits(auto &&container)
        {
            for (const auto &outfit : container)
            {
                m_outfitList.emplace_back(outfit);
            }
        }

        void RenameOutfit(const OutfitIterator &where, const std::string &&newName)
        {
            if (where != m_outfitList.end())
            {
                where->SetName(newName);
            }
        }

        void AddArmor(const OutfitIterator &where, Armor *armor)
        {
            if (where != m_outfitList.end())
            {
                where->AddArmor(armor);
            }
        }

        void AddArmors(const OutfitIterator &where, const std::vector<Armor *> &armors)
        {
            if (where != m_outfitList.end())
            {
                for (const auto &armor : armors)
                {
                    where->AddArmor(armor);
                }
            }
        }

        void DeleteOutfit(const OutfitConstIterator &where)
        {
            if (where != m_outfitList.cend())
            {
                m_outfitList.erase(where);
            }
        }

        void DeleteArmor(const OutfitIterator &where, const Armor *armor)
        {
            if (where != m_outfitList.end())
            {
                where->RemoveArmor(armor);
            }
        }

        [[nodiscard]] constexpr auto GetOutfitList() -> std::list<SosUiOutfit> &
        {
            return m_outfitList;
        }

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

        [[nodiscard]] auto GetErrorMessages() -> std::list<std::string> &
        {
            return m_errorMessages;
        }
    };
}

#endif // SOSUIDATA_H