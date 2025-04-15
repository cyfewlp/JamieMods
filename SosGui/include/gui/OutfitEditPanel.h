#pragma once

#include "common/config.h"
#include "coroutine.h"
#include "gui/Popup.h"
#include "gui/SosDataCoordinator.h"
#include "gui/Table.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    class OutfitEditPanel
    {
    public:
        using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor = RE::TESObjectARMO;

        static constexpr int MAX_FILTER_ARMOR_NAME = 256;
        static constexpr int SLOT_COUNT            = 32;
        static constexpr int SOS_SLOT_OFFSET       = 30;

    private:
        static void init_slot_name();

        std::string                             m_windowTitle;
        TableContext<3>                         m_armorListTable;
        TableContext<5>                         m_armorCandidatesTable;
        int                                     m_armorAddPolicy     = 0;
        SKSE::stl::enumeration<Slot, uint32_t>  m_selectedFilterSlot = Slot::kNone;
        bool                                    m_fFilterPlayable    = false;
        bool                                    m_fShowOutfitWindow  = false;
        std::array<char, MAX_FILTER_ARMOR_NAME> m_filterStringBuf;
        SosUiData                              &m_uiData;
        SosDataCoordinator                     &m_dataCoordinator;
        Armor                                  *m_selectedArmor = nullptr;
        Popup::DeleteArmorPopup                 m_DeleteArmorPopup;
        Popup::ConflictArmorPopup               m_ConflictArmorPopup;

    public:
        explicit OutfitEditPanel(SosUiData &uiData, SosDataCoordinator &dataCoordinator)
            : m_armorListTable(
                  TableContext<3>::Create("##OutfitArmors", {"$SosGui_TableHeader_Slot", "$ARMOR", "$Delete"})),
              m_armorCandidatesTable(TableContext<5>::Create(
                  "##ArmorCandidates", {"$ARMOR", "FormID", "ModName", "$SosGui_TableHeader_Slot", "$Add"})),
              m_uiData(uiData), m_dataCoordinator(dataCoordinator)
        {
            m_armorListTable.Resizable().SizingStretchProp();
            m_armorCandidatesTable.Resizable().Sortable();
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
            m_fShowOutfitWindow = show;
            UpdateWindowTitle(outfitName);
        }

    private:
        CoroutinePromise operator<<(CoroutineTask &&task) { co_await task; }

        void UpdateWindowTitle(const std::string &outfitName);

        void RenderProperties(const SosUiData::OutfitPair &wantEdit);

        void RenderArmorList(const SosUiData::OutfitPair &wantEdit);

        void RenderEditPanel(const SosUiData::OutfitPair &wantEdit);

        auto RenderArmorSlotFilter() -> bool;

        void RenderArmorCandidates(const SosUiData::OutfitPair &wantEdit);

        void RenderEditPanelPolicy(const SosUiData::OutfitPair &wantEdit);

        void RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit, const bool &fFilterPlayable);

        void UpdateArmorCandidates(const SosUiData::OutfitPair &wantEdit, bool mustBePlayable, OutfitAddPolicy policy);

        void UpdateArmorCandidatesBySlot(const SosUiData::OutfitPair &wantEdit, Slot slot) const;

        void UpdateArmorCandidatesForAny(const SosUiData::OutfitPair &wantEdit, bool mustBePlayable);

        auto IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool;

        void OnAddArmor(const SosUiData::OutfitPair &wantEdit, Armor *armor);

        void RenderPopups(const SosUiData::OutfitPair &wantEdit);
    };
}