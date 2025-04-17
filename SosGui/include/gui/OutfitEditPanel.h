#pragma once

#include "common/config.h"
#include "coroutine.h"
#include "gui/Popup.h"
#include "gui/SosDataCoordinator.h"
#include "gui/Table.h"
#include "util/PageUtil.h"
#include "widgets.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
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
            uint8_t                                 newFilterSlot      = SLOT_COUNT;
            std::array<uint16_t, SLOT_COUNT>        slotArmorCounter;
            bool                                    filterPlayable   = false;
            bool                                    showOutfitWindow = false;
            std::array<char, MAX_FILTER_ARMOR_NAME> filterStringBuf;
            // be used on click add(candidate table)/delete(armor table)
            Armor *selectedArmor = nullptr;
            // candidate armor variables
            MultiSelection  candidateSelection;
            SlotEnumeration candidateSelectedSlot; // be used to highlight conflict armors
        } m_editContext = {};

        std::string               m_windowTitle;
        TableContext<5>           m_armorListTable;
        TableContext<5>           m_armorCandidatesTable;
        SosUiData                &m_uiData;
        SosDataCoordinator       &m_dataCoordinator;
        Popup::DeleteArmorPopup   m_DeleteArmorPopup;
        Popup::ConflictArmorPopup m_ConflictArmorPopup;
        Popup::SlotPolicyHelp     m_slotPolicyHelp;
        Popup::BatchAddArmors     m_batchAddArmorsPopUp;
        util::PageUtil            m_armorCandidatesPage;

    public:
        explicit OutfitEditPanel(SosUiData &uiData, SosDataCoordinator &dataCoordinator)
            : m_armorListTable(
                  TableContext<5>::Create("##OutfitArmors", {"##Number", "$SosGui_TableHeader_Slot", "$ARMOR",
                                                             "$SkyOutSys_OEdit_OutfitSettings_Header", "$Delete"})),
              m_armorCandidatesTable(
                  TableContext<5>::Create("##ArmorCandidates", {"##Number", "$ARMOR", "FormID", "ModName", "$Add"})),
              m_uiData(uiData), m_dataCoordinator(dataCoordinator)
        {
            m_armorListTable.Resizable().SizingStretchProp();
            m_armorCandidatesTable.Resizable().Sortable().Hideable().Reorderable();
        }

        // return true if edit window closed;
        auto Render(const SosUiData::OutfitPair &wantEdit) -> bool;

        /// <summary>
        /// Show this outfit edit window with specificy outfit-name.
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

        auto RenderArmorSlotFilter() -> bool;

        void RenderArmorCandidates(const SosUiData::OutfitPair &wantEdit);

        static void CandidateContextMenu(bool &acceptAddAll);

        void BatchAddArmors(const SosUiData::OutfitPair &wantEdit);

        void RenderEditPanelPolicy(const SosUiData::OutfitPair &wantEdit);

        void RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit, const bool &fFilterPlayable);

        void UpdateArmorCandidates(const SosUiData::OutfitPair &wantEdit, bool mustBePlayable, OutfitAddPolicy policy);

        void UpdateArmorCandidatesBySlot(const SosUiData::OutfitPair &wantEdit, Slot slot);

        void UpdateArmorCandidatesForAny(const SosUiData::OutfitPair &wantEdit, bool mustBePlayable) const;

        static auto IsFilterArmor(const std::string_view &filterString, const Armor *armor) -> bool;

        void OnAddArmor(const SosUiData::OutfitPair &wantEdit, Armor *armor);

        void RenderPopups(const SosUiData::OutfitPair &wantEdit);

        static auto ToSlot(const uint8_t slotPos) -> Slot
        {
            return slotPos >= SLOT_COUNT ? Slot::kNone : static_cast<Slot>(1 << slotPos);
        }
    };

}