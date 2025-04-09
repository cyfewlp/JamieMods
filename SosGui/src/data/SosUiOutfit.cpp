#include "data/SosUiOutfit.h"
#include "common/config.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <cstdint>

namespace LIBC_NAMESPACE_DECL
{
    void SosUiOutfit::AddArmor(Armor *armor)
    {
        if (armor == nullptr)
        {
            return;
        }
        auto slot = armor->GetSlotMask();
        if (slot == Slot::kNone)
        {
            return;
        }
        for (uint32_t idx = 0; idx < SLOT_COUNT; ++idx)
        {
            if (armor->HasPartOf(static_cast<Slot>(1 << idx)))
            {
                m_armors[idx] = armor;
            }
        }
        m_slotMask.set(slot);
    }

    void SosUiOutfit::RemoveArmor(const Armor *armor)
    {
        if (armor == nullptr)
        {
            return;
        }
        auto slot = armor->GetSlotMask();
        if (slot == Slot::kNone)
        {
            return;
        }
        for (uint32_t idx = 0; idx < SLOT_COUNT; ++idx)
        {
            if (armor->HasPartOf(static_cast<Slot>(1 << idx)))
            {
                if (m_armors[idx] != armor)
                {
                    return;
                }
                m_armors[idx] = nullptr;
            }
        }
        m_slotMask.reset(slot);
    }

    auto SosUiOutfit::GetArmorAt(uint32_t slotPos) -> Armor *
    {
        if (slotPos == 0 || slotPos >= SLOT_COUNT)
        {
            return nullptr;
        }
        return m_armors.at(slotPos);
    }
}