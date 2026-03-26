#include "data/SosUiOutfit.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <cstdint>

template <>
struct std::hash<RE::TESObjectARMO *>
{
    _NODISCARD _STATIC_CALL_OPERATOR size_t operator()(const RE::TESObjectARMO *_Keyval) _CONST_CALL_OPERATOR noexcept
    {
        return _Hash_representation(_Keyval->GetFormID());
    }
};

namespace SosGui
{

void SosUiOutfit::AddArmor(const Armor *armor)
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

auto SosUiOutfit::GetArmorAt(uint32_t slotPos) const -> const Armor *
{
    if (slotPos >= SLOT_COUNT)
    {
        return nullptr;
    }
    return m_armors.at(slotPos);
}

auto SosUiOutfit::GetUniqueArmors() const -> std::unordered_set<const Armor *>
{
    std::unordered_set<const Armor *> ret;
    for (uint32_t idx = 0; idx < m_armors.size(); ++idx)
    {
        if (m_armors[idx] != nullptr)
        {
            ret.insert(m_armors[idx]);
        }
    }
    return ret;
}
} // namespace SosGui
