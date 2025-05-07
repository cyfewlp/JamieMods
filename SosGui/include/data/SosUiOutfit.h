#pragma once

#include "common/config.h"
#include "data/id.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cstdint>
#include <string>
#include <unordered_set>

namespace LIBC_NAMESPACE_DECL
{
    class SosUiOutfit
    {
    public:
        static constexpr int SLOT_COUNT = 32;
        using Slot                      = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor                     = RE::TESObjectARMO;
        using SlotPolicyArray           = std::array<std::string, SLOT_COUNT>;

    private:
        OutfitId                               m_id;
        std::string                            m_name;
        SKSE::stl::enumeration<Slot, uint32_t> m_slotMask = Slot::kNone;
        std::array<Armor *, SLOT_COUNT>        m_armors;
        SlotPolicyArray                        m_slotPolicies;
        bool                                   m_isFavorite = false;

    public:
        explicit SosUiOutfit(OutfitId id, const std::string &name, bool favorite = false)
            : m_id(id), m_name(name), m_isFavorite(favorite)
        {
            m_armors.fill(nullptr);
        }

        ~SosUiOutfit() = default;

        [[nodiscard]] constexpr auto GetId() const -> OutfitId
        {
            return m_id;
        }

        void AddArmor(Armor *armor);

        void RemoveArmor(const Armor *armor);

        auto GetArmorAt(uint32_t slotPos) const -> Armor *;

        auto HasSlot(uint32_t slotPos) const -> bool
        {
            return m_slotMask.all(static_cast<Slot>(1 << slotPos));
        }

        auto IsConflictWith(const Armor *armor) const -> bool
        {
            return m_slotMask.any(armor->GetSlotMask());
        }

        void SetName(const std::string &newName)
        {
            m_name.assign(newName);
        }

        void SetFavorite(bool isFavorite)
        {
            m_isFavorite = isFavorite;
        }

        [[nodiscard]] constexpr auto IsFavorite() const -> bool
        {
            return m_isFavorite;
        }

        [[nodiscard]] auto GetName() const -> const std::string &
        {
            return m_name;
        }

        [[nodiscard]] auto IsEmpty() const -> bool
        {
            return m_slotMask.underlying() == 0;
        }

        template <typename string>
        auto SetSlotPolicies(uint32_t slotPos, string &&policy)
        {
            if (slotPos >= SLOT_COUNT)
            {
                return;
            }
            m_slotPolicies.at(slotPos) = std::forward<string>(policy);
        }

        [[nodiscard]] auto GetSlotPolicies() const -> const SlotPolicyArray &
        {
            return m_slotPolicies;
        }

        auto GetUniqueArmors() const -> std::unordered_set<Armor *>;
    };

    constexpr auto GetOutfitId(const SosUiOutfit &outfit)
    {
        return outfit.GetId();
    }

    constexpr auto GetOutfitName(const SosUiOutfit &outfit)
    {
        return outfit.GetName();
    }
}