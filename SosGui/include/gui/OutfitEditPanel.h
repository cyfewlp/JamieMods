#pragma once

#include "BaseGui.h"
#include "common/config.h"
#include "data/ArmorGenerator.h"
#include "data/ArmorContainer.h"
#include "data/SosUiData.h"
#include "gui/Popup.h"
#include "util/ImGuiUtil.h"
#include "service/OutfitService.h"
#include "util/PageUtil.h"
#include "widgets.h"

#include "data/SosUiOutfit.h"
#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
class OutfitEditPanel final : public BaseGui
{
public:
    using Slot = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor = RE::TESObjectARMO;
    using SlotEnumeration = SKSE::stl::enumeration<Slot, uint32_t>;

    static constexpr int MAX_FILTER_ARMOR_NAME = 256;
    static constexpr int SLOT_COUNT = 32;
    static constexpr int SOS_SLOT_OFFSET = 30;

private:
    static auto get_slot_name_key(uint32_t slotPos) -> std::string;

    static bool IsArmorNonPlayable(const Armor *armor)
    {
        return (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0;
    }

    struct ArmorFilter final : ImGuiUtil::DebounceInput
    {
        bool mustPlayable = false;

        bool PassFilter(const Armor *armor) const;

        bool Draw();
    };

    enum class error : uint8_t
    {
        unassociated_armor,
        armor_already_exists,
    };

    struct ArmorView
    {
        std::bitset<RE::BIPED_OBJECT::kEditorTotal> selectedFilterSlot{};
        ArmorContainer armorContainer{};
        std::vector<Armor *> viewData{};
        ArmorFilter armorFilter{};
        bool checkAllSlot = true; // default shows all armor slot
        std::array<uint16_t, SLOT_COUNT> slotCounter{};
        MultiSelection multiSelection{};
        SlotEnumeration multiSelectedSlot{}; // be used to highlight conflict armors
        uint32_t availableArmorCount = 0;
        int addPolicy = 0;

        void draw_slot_filterer();

        //////////////////////////////////
        void init();
        void clear();
        void add_armors_by_policy();
        void add_armors_has_slot(Slot newSlot);
        [[nodiscard]] auto add_armor(Armor *armor) -> std::expected<void, error>;
        void remove_armors_has_slot(Slot selectedSlots, Slot toRemoveSlot);
        void add_armors_in_outfit(const SosUiOutfit *editingOutfit);
        void remove_armors_in_outfit(const SosUiOutfit *editingOutfit);
        void filter_reset(const SosUiOutfit *editingOutfit);
        void slot_counter_add(const Armor *armor);
        void slot_counter_remove(const Armor *armor);
        bool eraseArmor(const Armor *armor);
    } m_armorView = {};

    struct EditContext
    {
        bool armorListShowAllSlotArmors = false;
        // be used on click add(candidate table)/delete(armor table)
        bool dirty = true;
        void Clear();
    } m_editContext = {};

    std::string m_windowTitle;
    OutfitService &m_outfitService;
    Popup::DeleteArmorPopup m_DeleteArmorPopup{};
    Popup::ConflictArmorPopup m_ConflictArmorPopup{};
    Popup::SlotPolicyHelp m_slotPolicyHelp{};
    Popup::BatchAddArmors m_batchAddArmorsPopUp{};

public:
    explicit OutfitEditPanel(OutfitService &outfitService) : m_outfitService(outfitService) {}

    // return true if edit window closed;
    void Render(const SosUiData::OutfitPair &wantEdit);
    void Refresh() override;
    void Close() override;
    void OnSelectActor(const RE::Actor *actor, const SosUiOutfit *editingOutfit);
    void OnSelectOutfit(const SosUiOutfit *lastEditOutfit, const SosUiOutfit *editingOutfit);

private:
    void UpdateWindowTitle(const std::string &outfitName);

    void DrawOutfitArmors(const SosUiData::OutfitPair &wantEdit);
    void HighlightConflictSlot(Slot slot) const;
    void SlotPolicyCombo(const SosUiData::OutfitPair &wantEdit, const uint32_t &slotIdx) const;

    void RenderEditPanel(const SosUiData::OutfitPair &wantEdit);

    void DrawArmorView(const SosUiData::OutfitPair &wantEdit);

    void BatchAddArmors(const SosUiData::OutfitPair &wantEdit);

    void RenderEditPanelPolicy(const SosUiData::OutfitPair &wantEdit);

    void RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit, const bool &fFilterPlayable) const;

    static void GetArmorGeneratorFromPolicy(OutfitAddPolicy policy, ArmorGenerator **generator);

    //////////////////////////////////////////////////////////////////////////
    // View Functions
    //////////////////////////////////////////////////////////////////////////


    void OnAddArmor(const SosUiData::OutfitPair &wantEdit, Armor *armor);

    void RenderPopups(const SosUiData::OutfitPair &wantEdit);

    static auto IsArmorCanDisplay(const Armor *armor) -> bool;

    static auto ToSlot(uint32_t slotPos) -> Slot
    {
        return slotPos >= RE::BIPED_OBJECT::kEditorTotal ? Slot::kNone : static_cast<Slot>(1 << slotPos);
    }

    static auto ToSlot(const RE::BIPED_OBJECT equipIndex) -> Slot
    {
        if (equipIndex >= RE::BIPED_OBJECT::kEditorTotal)
        {
            return Slot::kNone;
        }
        return static_cast<Slot>(1 << equipIndex);
    }
};

}