#pragma once

#include "BaseGui.h"
#include "common/config.h"
#include "data/ArmorGenerator.h"
#include "data/ArmorView.h"
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
#include <basetsd.h>
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
    static void init_slot_name();

    static bool IsArmorNonPlayable(const Armor *armor)
    {
        return (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0;
    }

    struct ArmorFilter final : ImGuiUtil::debounce_input
    {
        bool mustPlayable = false;

        ArmorFilter() : debounce_input("##ArmorFilter",
                                       Translation::Translate("$SkyOutSys_OEdit_AddFromList_Filter_Name")) {}

        bool PassFilter(const Armor *armor) const;

        bool draw() override;
    };

    struct EditContext
    {
        int armorAddPolicy = 0;
        SlotEnumeration selectedFilterSlot = Slot::kNone;
        bool checkSlotAll = true; // default shows all armor slot
        bool prevCheckAllSlot = true;
        ArmorView armorView{};
        std::vector<Armor *> armorViewData;
        ArmorFilter armorFilter{};
        std::array<uint16_t, SLOT_COUNT> slotArmorCounter;
        bool showOutfitWindow = false;
        // be used on click add(candidate table)/delete(armor table)
        Armor *selectedArmor = nullptr;
        // candidate armor variables
        MultiSelection armorMultiSelection;
        SlotEnumeration candidateSelectedSlot{}; // be used to highlight conflict armors
        bool dirty = true;

        void Clear();
    } m_editContext = {};

    std::string m_windowTitle;
    OutfitService &m_outfitService;
    Popup::DeleteArmorPopup m_DeleteArmorPopup{};
    Popup::ConflictArmorPopup m_ConflictArmorPopup{};
    Popup::SlotPolicyHelp m_slotPolicyHelp{};
    Popup::BatchAddArmors m_batchAddArmorsPopUp{};
    UINT32 m_availableArmorCount = 0;

public:
    explicit OutfitEditPanel(OutfitService &outfitService) : m_outfitService(outfitService) {}

    // return true if edit window closed;
    auto Render(const SosUiData::OutfitPair &wantEdit) -> bool;

    /// <summary>
    /// Show this outfit edit window with specify outfit-name.
    /// </summary>
    /// <param name="outfitName">be used to set window title</param>
    /// <param name="show">true, show window</param>
    void ShowWindow(const std::string &outfitName, bool show = true)
    {
        m_editContext.showOutfitWindow = show;
        UpdateWindowTitle(outfitName);
    }

    void Refresh() override;
    void Close() override;
    void OnSelectActor(const RE::Actor *actor, const SosUiOutfit *editingOutfit);
    void OnSelectOutfit(const SosUiOutfit *lastEditOutfit, const SosUiOutfit *editingOutfit);

private:
    enum class error : uint8_t
    {
        unassociated_armor,
        armor_already_exists,
    };

    void UpdateWindowTitle(const std::string &outfitName);

    void RenderProperties(const SosUiData::OutfitPair &wantEdit) const;

    void RenderArmorList(const SosUiData::OutfitPair &wantEdit);
    void HighlightConflictArmor(const Armor *armor) const;
    void SlotPolicyCombo(const SosUiData::OutfitPair &wantEdit, const uint32_t &slotIdx) const;

    void RenderEditPanel(const SosUiData::OutfitPair &wantEdit);

    void RenderArmorSlotFilter(const SosUiData::OutfitPair &wantEdit);

    void RenderArmorCandidates(const SosUiData::OutfitPair &wantEdit);

    static void CandidateContextMenu(bool &acceptAddAll);

    void BatchAddArmors(const SosUiData::OutfitPair &wantEdit);

    void RenderEditPanelPolicy(const SosUiData::OutfitPair &wantEdit);

    void RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit, const bool &fFilterPlayable) const;

    void GetArmorGeneratorFromPolicy(ArmorGenerator **generator) const;

    //////////////////////////////////////////////////////////////////////////
    // View Functions
    //////////////////////////////////////////////////////////////////////////

    // Add all can display armors to ArmorView.
    void view_init();
    void view_add_armors_by_policy();
    void view_add_armors_has_slot(RE::BIPED_OBJECT equipIndex);
    [[nodiscard]] auto view_add_armor(Armor *armor) -> std::expected<void, error>;
    void view_remove_armors_has_slot(Slot selectedSlots, RE::BIPED_OBJECT equipIndex);
    // remove armors that already exists in outfit
    void view_add_armors_in_outfit(const SosUiOutfit *editingOutfit);
    void view_remove_armors_in_outfit(const SosUiOutfit *editingOutfit);
    // reset view by filterer
    void view_filter_reset(const SosUiOutfit *editingOutfit);
    void slotCounterAdd(const Armor *armor);
    void slotCounterRemove(const Armor *armor);

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

    bool eraseArmor(const Armor *armor);
};

}