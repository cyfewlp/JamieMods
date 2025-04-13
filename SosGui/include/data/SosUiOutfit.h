#pragma once

#include "common/config.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cstdint>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
    class SosUiOutfit
    {
    public:
        static constexpr int SLOT_COUNT = 32;
        using Slot                      = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor                     = RE::TESObjectARMO;

    private:
        std::string                            m_name;
        SKSE::stl::enumeration<Slot, uint32_t> m_slotMask = Slot::kNone;
        std::array<Armor *, SLOT_COUNT>        m_armors;

    public:
        explicit SosUiOutfit(const std::string &name) : m_name(name)
        {
            m_armors.fill(nullptr);
        }

        ~SosUiOutfit() = default;

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

        [[nodiscard]] auto GetName() const -> const std::string &
        {
            return m_name;
        }

        [[nodiscard]] auto IsEmpty() const -> bool
        {
            return m_slotMask.underlying() == 0;
        }
    };
}