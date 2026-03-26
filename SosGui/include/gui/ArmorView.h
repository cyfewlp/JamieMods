//
// Created by jamie on 2025/5/7.
//

#ifndef ARMORVIEW_H
#define ARMORVIEW_H

#include "common/config.h"
#include "data/ArmorContainer.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "util/ImGuiUtil.h"
#include "widgets.h"

namespace LIBC_NAMESPACE_DECL
{
struct ArmorView final
{
    using Slot            = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor           = RE::TESObjectARMO;
    using SlotEnumeration = SKSE::stl::enumeration<Slot, uint32_t>;

    static bool IsArmorNonPlayable(const Armor *armor) { return (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0; }

    struct ArmorFilter final : ImGuiUtil::DebounceInput
    {
        bool mustPlayable = false;

        bool PassFilter(const Armor *armor) const;

        bool Draw();
    };

    struct ModFilterer
    {
        std::vector<std::pair<std::string_view, bool>> passModList;

        bool PassFilter(const Armor *armor) const;

        void Clear() { passModList.clear(); }
    };

    struct ArmorMultiSelection : MultiSelection
    {
        SlotEnumeration slotMask = Slot::kNone;

        auto UpdateSelected(const Armor *armor, const size_t index) -> bool
        {
            bool con = Contains(static_cast<ImGuiID>(index));
            if (con)
                slotMask.set(armor->GetSlotMask());
            else
                slotMask.reset(armor->GetSlotMask());
            return con;
        }

        auto IsSelectSlot(Slot slot) const -> bool { return slotMask.all(slot); }

        void Clear()
        {
            MultiSelection::Clear();
            slotMask = Slot::kNone;
        }
    };

    static constexpr uint32_t SLOT_COUNT = RE::BIPED_OBJECT::kEditorTotal;

    // A wrap class. Be used to cache armor rank(sort by name).
    class RankedArmor
    {
        const Armor *armor;
        size_t       rank = -1;

    public:
        explicit RankedArmor(const Armor *const armor, const size_t rank) : armor(armor), rank(rank) {}

        [[nodiscard]] auto Rank() const -> size_t { return rank; }

        auto operator->() const -> const Armor * { return armor; }

        operator const Armor *() const { return armor; }
    };

private:
    std::vector<RankedArmor> viewData{};

public:
    std::bitset<SLOT_COUNT>                        slotFiltererSelected{};
    uint32_t                                       availableArmorCount = 0;
    ArmorGenerator                                *armorGenerator      = nullptr;
    std::array<uint16_t, SLOT_COUNT>               slotCounter{};
    ArmorContainer                                 armorContainer{};
    std::unordered_map<std::string_view, uint32_t> modRefCounter; // only update when generator update
    ArmorFilter                                    armorFilter{};
    ModFilterer                                    modFilterer{};
    ArmorMultiSelection                            multiSelection{};
    bool                                           checkAllSlot = true; // default shows all armor slot

    enum class error : uint8_t
    {
        unassociated_armor,
        armor_already_exists,
        armor_not_exist,
    };

    static auto ToErrorMessage(const error error) -> std::string
    {
        switch (error)
        {
            case error::unassociated_armor:
                return "Unassociated Armor: Missing in container, try reopen menu to resolve.";
            case error::armor_already_exists:
                return "Armor already exists in view";
            case error::armor_not_exist:
                return "Armor not exist in view";
            default:
                return "Unknown error";
        }
    }

    ////////////////////////////////////////////////////////////////////
    // mod filterer -> slot-filterer -> armor-name filter
    void               init();
    void               on_refresh();
    void               clear();
    void               clearViewData();
    void               remove_armors_has_slot(Slot selectedSlots, Slot toRemoveSlot);
    void               add_armors_in_outfit(SosUiData &uiData, const SosUiOutfit *editingOutfit);
    void               remove_armors_in_outfit(const SosUiOutfit *editingOutfit);
    bool               filter(const Armor *armor) const;
    [[nodiscard]] auto add_armor(const Armor *armor) -> std::expected<void, error>;
    bool               remove_armor(const Armor *armor);
    void               reset_counter();
    void               update_view_data(ArmorGenerator *generator, const SosUiOutfit *editingOutfit);
    void               reset_view(ArmorGenerator *generator, const SosUiOutfit *editingOutfit);
    auto               find_armor(const Armor *armor) const -> std::expected<size_t, error>;
    bool               no_select_any_slot() const;

    [[nodiscard]] auto ViewData() const -> const std::vector<RankedArmor> & { return viewData; }

    auto SwapViewData(std::vector<RankedArmor> &other) -> void { viewData.swap(other); }
};
} // namespace LIBC_NAMESPACE_DECL

#endif // ARMORVIEW_H
