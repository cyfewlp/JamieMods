#pragma once

#include "common/config.h"
#include "coroutine.h"
#include "data/ArmorGenerator.h"
#include "data/ArmorView.h"
#include "data/SosUiData.h"
#include "gui/Popup.h"
#include "gui/Table.h"
#include "service/OutfitService.h"
#include "util/PageUtil.h"
#include "widgets.h"

#include "data/SosUiOutfit.h"
#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <basetsd.h>
#include <cstdint>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    class OutfitEditPanel
    {
    public:
        using Slot            = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor           = RE::TESObjectARMO;
        using SlotEnumeration = SKSE::stl::enumeration<Slot, uint32_t>;

        static constexpr int MAX_FILTER_ARMOR_NAME = 256;
        static constexpr int SLOT_COUNT            = 32;
        static constexpr int SOS_SLOT_OFFSET       = 30;

    private:
        static void init_slot_name();

        struct EditContext
        {
            int                                     armorAddPolicy     = 0;
            SlotEnumeration                         selectedFilterSlot = Slot::kNone;
            bool                                    checkSlotAll       = true; // default shows all armor slot
            bool                                    prevCheckAllSlot   = true;
            std::array<char, MAX_FILTER_ARMOR_NAME> filterStringBuf;
            ArmorView                               armorView1{};
            ArmorView                               armorView2{}; // filter by armor name, mod name, playable
            std::array<uint16_t, SLOT_COUNT>        slotArmorCounter;
            bool                                    filterPlayable   = false;
            bool                                    showOutfitWindow = false;
            // be used on click add(candidate table)/delete(armor table)
            Armor *selectedArmor = nullptr;
            // candidate armor variables
            MultiSelection  candidateSelection;
            SlotEnumeration candidateSelectedSlot{}; // be used to highlight conflict armors
        } m_editContext = {};

        std::string               m_windowTitle;
        TableContext<5>           m_armorListTable;
        TableContext<5>           m_armorCandidatesTable;
        OutfitService            &m_outfitService;
        Popup::DeleteArmorPopup   m_DeleteArmorPopup{};
        Popup::ConflictArmorPopup m_ConflictArmorPopup{};
        Popup::SlotPolicyHelp     m_slotPolicyHelp{};
        Popup::BatchAddArmors     m_batchAddArmorsPopUp{};
        util::PageUtil            m_armorCandidatesPage{};
        UINT32                    m_availableArmorCount;

    public:
        explicit OutfitEditPanel(OutfitService &outfitService)
            : m_armorListTable(
                  TableContext<5>::Create("##OutfitArmors", {"##Number", "$SosGui_TableHeader_Slot", "$ARMOR",
                                                             "$SkyOutSys_OEdit_OutfitSettings_Header", "$Delete"})),
              m_armorCandidatesTable(
                  TableContext<5>::Create("##ArmorCandidates", {"##Number", "$ARMOR", "FormID", "ModName", "$Add"})),
              m_outfitService(outfitService)
        {
            m_armorListTable.Resizable().SizingStretchProp();
            m_armorCandidatesTable.Resizable().Sortable().Hideable().Reorderable();
        }

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

    private:
        CoroutinePromise operator<<(CoroutineTask &&task) const
        {
            co_await task;
        }

        void UpdateWindowTitle(const std::string &outfitName);

        void RenderProperties(const SosUiData::OutfitPair &wantEdit);

        void RenderArmorList(const SosUiData::OutfitPair &wantEdit);
        void HighlightConflictArmor(Armor *armor) const;
        void SlotPolicyCombo(const SosUiData::OutfitPair &wantEdit, const uint32_t &slotIdx) const;

        void RenderEditPanel(const SosUiData::OutfitPair &wantEdit);

        void RenderArmorSlotFilter(const SosUiData::OutfitPair &wantEdit);

        void RenderArmorCandidates(const SosUiData::OutfitPair &wantEdit);

        static void CandidateContextMenu(bool &acceptAddAll);

        void BatchAddArmors(const SosUiData::OutfitPair &wantEdit);

        void RenderEditPanelPolicy(const SosUiData::OutfitPair &wantEdit);

        void RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit, const bool &fFilterPlayable);

        void GetArmorGeneratorFromPolicy(ArmorGenerator **generator) const;

        //////////////////////////////////////////////////////////////////////////
        // View Functions
        //////////////////////////////////////////////////////////////////////////

        void view_add_armors_by_policy(const SosUiOutfit *outfit);
        void view_add_armors_has_slot(RE::BIPED_OBJECT equipIndex);

        template <typename Generator>
        void view_add_armors_with_generator(Generator &&generator)
        {
            m_availableArmorCount = 0;
            m_editContext.armorView1.Clear();
            generator.for_each([&](Armor *armor) {
                m_editContext.armorView1.Insert(armor);
                m_availableArmorCount++;
            });
        }

        void view_remove_armors_has_slot(Slot selectedSlots, RE::BIPED_OBJECT equipIndex);
        // remove armors that already exists in outfit
        void view_remove_armors_in_outfit(const SosUiOutfit *editingOutfit);
        // remove armors from current view by filterer;
        void view_filter_remove();
        // reset view by filterer
        void view_filter_reset(const SosUiOutfit *editingOutfit);
        auto IsFilterArmor(const Armor *armor) -> bool;

        void OnAddArmor(const SosUiData::OutfitPair &wantEdit, Armor *armor);

        void RenderPopups(const SosUiData::OutfitPair &wantEdit);

        auto IsArmorCanDisplay(Armor *armor) const -> bool;

        static auto ToSlot(uint32_t slotPos) -> Slot
        {
            return slotPos >= RE::BIPED_OBJECT::kEditorTotal ? Slot::kNone : static_cast<Slot>(1 << slotPos);
        }

        static auto ToSlot(const RE::BIPED_OBJECT equipIndex) -> Slot
        {
            if (equipIndex >= RE::BIPED_OBJECT::kEditorTotal || equipIndex == RE::BIPED_OBJECT::kNone)
            {
                return Slot::kNone;
            }
            return static_cast<Slot>(1 << equipIndex);
        }
    };

}