//
// Created by jamie on 2025/4/3.
//

#ifndef SOSUIDATA_H
#define SOSUIDATA_H

#pragma once

#include "SosDataType.h"
#include "common/config.h"
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
        using Armor         = RE::TESObjectARMO;
        using BodySlot      = int32_t;
        using OutfitState   = std::pair<StateType, std::string>;
        using BodySlotArmor = std::pair<BodySlot, Armor *>;

    private:
        std::vector<RE::Actor *>                     m_actors;
        std::vector<RE::Actor *>                     m_NearActors;
        bool                                         m_enabled           = false;
        bool                                         m_fQuickSlotEnabled = false;
        std::vector<RE::TESObjectARMO *>             m_armorCandidates;
        std::vector<RE::TESObjectARMO *>             m_armorCandidatesCopy;
        std::unordered_map<RE::Actor *, std::string> m_actorActiveOutfitMap;
        std::unordered_map<std::string, SosUiOutfit> m_outfitMap;
        std::list<std::string>                       m_errorMessages;

        std::unordered_map<RE::Actor *, bool>                                       m_autoSwitchEnabled;
        std::unordered_map<RE::Actor *, std::unordered_map<StateType, std::string>> m_actorOutfitStates;

    public:
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
            if (actor != nullptr && std::find(m_actors.begin(), m_actors.end(), actor) == m_actors.end())
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

        [[nodiscard]] constexpr auto GetArmorCandidates() -> std::vector<RE::TESObjectARMO *> &
        {
            return m_armorCandidates;
        }

        [[nodiscard]] constexpr auto GetArmorCandidatesCopy() -> std::vector<RE::TESObjectARMO *> &
        {
            return m_armorCandidatesCopy;
        }

        void DeleteCandidateArmor(Armor *armor)
        {
            std::erase(m_armorCandidatesCopy, armor);
        }

        void SetArmorCandidates(const std::vector<Armor *> &armorCandidates)
        {
            m_armorCandidates.clear();
            m_armorCandidatesCopy.clear();
            for (const auto &outfit : armorCandidates)
            {
                m_armorCandidates.push_back(outfit);
                m_armorCandidatesCopy.push_back(outfit);
            }
        }

        void ResetArmorCandidatesCopy()
        {
            m_armorCandidatesCopy.clear();
            for (const auto &outfit : m_armorCandidates)
            {
                m_armorCandidatesCopy.push_back(outfit);
            }
        }

        ////////////////////////////////////////////////////////////////////////////
        // SosUiOutfit
        ////////////////////////////////////////////////////////////////////////////

        void AddOutfit(const std::string &outfitName)
        {
            m_outfitMap.emplace(outfitName, SosUiOutfit(outfitName));
        }

        void RenameOutfit(std::string &&outfitName, std::string &&newName)
        {
            if (m_outfitMap.contains(outfitName))
            {
                auto &outfit = m_outfitMap.at(outfitName);
                m_outfitMap.erase(outfitName);
                outfit.SetName(newName);
                m_outfitMap.at(newName) = outfit;
            }
        }

        void DeleteOutfit(const std::string &outfitName)
        {
            m_outfitMap.erase(outfitName);
        }

        void AddArmor(const std::string &&outfitName, Armor *armor)
        {
            if (m_outfitMap.contains(outfitName))
            {
                auto &outfit = m_outfitMap.at(outfitName);
                outfit.AddArmor(armor);
            }
        }

        void DeleteArmor(const std::string &&outfitName, Armor *armor)
        {
            if (m_outfitMap.contains(outfitName))
            {
                auto &outfit = m_outfitMap.at(outfitName);
                outfit.RemoveArmor(armor);
            }
        }

        [[nodiscard]] auto GetOutfitMap() -> std::unordered_map<std::string, SosUiOutfit> &
        {
            return m_outfitMap;
        }

        [[nodiscard]] auto GetActorActiveOutfitMap() const -> const std::unordered_map<RE::Actor *, std::string> &
        {
            return m_actorActiveOutfitMap;
        }

        void SetActorActiveOutfit(RE::Actor *actor, const std::string &outfitName)
        {
            m_actorActiveOutfitMap[actor] = outfitName;
        }

        auto HasActiveOutfit(RE::Actor *actor) -> bool
        {
            return m_actorActiveOutfitMap.contains(actor) && !m_actorActiveOutfitMap.at(actor).empty();
        }

        auto IsActorActiveOutfit(RE::Actor *actor, const std::string &outfitName) -> bool
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
