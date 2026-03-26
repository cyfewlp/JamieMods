#pragma once

#include "Translation.h"
#include "data/id.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <string>
#include <unordered_set>

namespace SosGui
{
class SosUiOutfit
{
public:
    static constexpr int SLOT_COUNT = RE::BIPED_OBJECT::kEditorTotal;
    using Slot                      = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor                     = RE::TESObjectARMO;
    using SlotPolicyArray           = std::array<std::string, SLOT_COUNT>;

private:
    OutfitId                               m_id;
    std::string                            m_name;
    SKSE::stl::enumeration<Slot, uint32_t> m_slotMask = Slot::kNone;
    std::array<const Armor *, SLOT_COUNT>  m_armors;
    SlotPolicyArray                        m_slotPolicies;
    bool                                   m_isFavorite = false;

public:
    explicit constexpr SosUiOutfit(OutfitId id, const std::string &name, bool favorite = false) : m_id(id), m_name(name), m_isFavorite(favorite)
    {
        m_armors.fill(nullptr);
    }

    [[nodiscard]] constexpr auto GetId() const -> OutfitId { return m_id; }

    void AddArmor(const Armor *armor);

    void RemoveArmor(const Armor *armor);

    auto GetArmorAt(uint32_t slotPos) const -> const Armor *;

    auto HasSlot(uint32_t slotPos) const -> bool { return m_slotMask.all(static_cast<Slot>(1 << slotPos)); }

    auto IsConflictWith(const Armor *armor) const -> bool { return m_slotMask.any(armor->GetSlotMask()); }

    void SetName(const std::string &newName) { m_name.assign(newName); }

    void SetFavorite(bool isFavorite) { m_isFavorite = isFavorite; }

    [[nodiscard]] constexpr auto IsFavorite() const -> bool { return m_isFavorite; }

    [[nodiscard]] auto GetName() const -> const std::string & { return m_name; }

    [[nodiscard]] auto IsEmpty() const -> bool { return m_slotMask.underlying() == 0; }

    void SetSlotPolicies(const uint32_t slotPos, const std::string &policy)
    {
        if (slotPos >= SLOT_COUNT)
        {
            return;
        }
        m_slotPolicies.at(slotPos) = policy;
    }

    [[nodiscard]] auto GetSlotPolicies() const -> const SlotPolicyArray & { return m_slotPolicies; }

    auto GetUniqueArmors() const -> std::unordered_set<const Armor *>;
};

static constexpr OutfitId UNTITLED_OUTFIT_ID = INVALID_OUTFIT_ID;

class EditingOutfit
{
    const SosUiOutfit *m_sourceOutfit;

public:
    explicit EditingOutfit(const SosUiOutfit &outfit) : m_sourceOutfit(&outfit) {}

    explicit EditingOutfit(const SosUiOutfit *pOutfit) : m_sourceOutfit(pOutfit) {}

    [[nodiscard]] auto GetSourceOutfit() const -> const SosUiOutfit * { return m_sourceOutfit; }

    auto operator->() const -> const SosUiOutfit * { return m_sourceOutfit; }

    auto GetId() const -> OutfitId { return m_sourceOutfit->GetId(); }

    bool IsUntitled() const { return GetId() == UNTITLED_OUTFIT_ID; }

    auto GetName() const
    {
        if (!m_sourceOutfit || IsUntitled()) return Translation::Translate("$SosGui_Untitled");
        return m_sourceOutfit->GetName();
    }

    bool IsConflictWith(const RE::TESObjectARMO *armor) const { return m_sourceOutfit->IsConflictWith(armor); }

    bool IsEmpty() const { return m_sourceOutfit->IsEmpty(); }

    auto GetArmorAt(uint32_t slotIdx) const -> const RE::TESObjectARMO * { return m_sourceOutfit->GetArmorAt(slotIdx); }

    auto GetSlotPolicies() const -> const SosUiOutfit::SlotPolicyArray & { return m_sourceOutfit->GetSlotPolicies(); }
};

constexpr auto GetOutfitId(const SosUiOutfit &outfit)
{
    return outfit.GetId();
}

constexpr auto GetOutfitName(const SosUiOutfit &outfit)
{
    return outfit.GetName();
}
} // namespace SosGui
