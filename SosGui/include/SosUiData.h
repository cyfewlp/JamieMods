//
// Created by jamie on 2025/4/3.
//

#ifndef SOSUIDATA_H
#define SOSUIDATA_H

#pragma once

#include "RE/A/Actor.h"
#include "SosDataType.h"
#include "common/config.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    class SosUiData
    {
    public:
        using BodySlot      = int32_t;
        using OutfitState   = std::pair<StateType, RE::BSFixedString>;
        using BodySlotArmor = std::pair<BodySlot, RE::TESObjectARMO *>;

    private:
        std::vector<RE::Actor *>                                    m_actors;
        std::vector<RE::Actor *>                                    m_NearActors;
        bool                                                        m_enabled           = false;
        bool                                                        m_fQuickSlotEnabled = false;
        std::unordered_map<RE::Actor *, bool>                       m_autoSwitchEnabled;
        std::unordered_map<RE::Actor *, OutfitState>                m_actorOutfitStates;
        std::unordered_map<std::string, std::vector<BodySlotArmor>> m_outfitBodySlotArmors;
        std::vector<std::string>                                    m_outfitList;
        std::vector<RE::TESObjectARMO *>                            m_armorCandidates;
        std::vector<RE::TESObjectARMO *>                            m_armorCandidatesCopy;

    public:
        static auto GetInstance() -> SosUiData &
        {
            static SosUiData instance;
            return instance;
        }

        [[nodiscard]] auto GetActors() const -> const std::vector<RE::Actor *> &
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

        void SetNearActors(const std::vector<RE::Actor *> &nearActors)
        {
            m_NearActors.clear();
            for (const auto &actor : nearActors)
            {
                m_NearActors.push_back(actor);
            }
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

        [[nodiscard]] auto GetActorOutfitStates() const -> const std::unordered_map<RE::Actor *, OutfitState> &
        {
            return m_actorOutfitStates;
        }

        void PutActorOutfitState(RE::Actor *actor, OutfitState &&state)
        {
            m_actorOutfitStates[actor] = std::forward<OutfitState>(state);
        }

        [[nodiscard]] constexpr auto GetOutfitList() const -> const std::vector<std::string> &
        {
            return m_outfitList;
        }

        void SetOutfitList(const std::vector<std::string> &outfitLists)
        {
            m_outfitList.clear();
            for (const auto &outfit : outfitLists)
            {
                m_outfitList.push_back(outfit);
            }
        }

        [[nodiscard]] constexpr auto GetArmorCandidates() -> std::vector<RE::TESObjectARMO *> &
        {
            return m_armorCandidates;
        }

        [[nodiscard]] constexpr auto GetArmorCandidatesCopy() -> std::vector<RE::TESObjectARMO *> &
        {
            return m_armorCandidatesCopy;
        }

        void SetArmorCandidates(const std::vector<RE::TESObjectARMO *> &armorCandidates)
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

        [[nodiscard]] auto GetOutfitBodySlotArmors() const
            -> const std::unordered_map<std::string, std::vector<BodySlotArmor>> &
        {
            return m_outfitBodySlotArmors;
        }

        void SetOutfitBodySlotArmors(std::string &outfitName, std::vector<int32_t> slots,
                                     std::vector<RE::TESObjectARMO *> armors)
        {
            m_outfitBodySlotArmors.erase(outfitName);
            std::vector<BodySlotArmor> bodySlotArmors;
            int                        size = armors.size();
            for (int idx = 0; idx < size; ++idx)
            {
                bodySlotArmors.emplace_back(slots[idx], armors[idx]);
            }
            m_outfitBodySlotArmors[outfitName] = bodySlotArmors;
        }
    };
}

#endif // SOSUIDATA_H
