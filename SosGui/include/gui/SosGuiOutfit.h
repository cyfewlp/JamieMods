#pragma once

#include "ImGuiUtil.h"
#include "SosUiData.h"
#include "Translation.h"
#include "common/config.h"
#include "data/SosUiOutfit.h"
#include "gui/SosDataCoordinator.h"
#include "gui/Popup.h"
#include "gui/Table.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <imgui.h>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    class SosGuiOutfit
    {
        using Slot  = RE::BIPED_MODEL::BipedObjectSlot;
        using Armor = RE::TESObjectARMO;

        static constexpr int MAX_FILTER_ARMOR_NAME = 256;
        static constexpr int SLOT_COUNT            = 32;
        static constexpr int SOS_SLOT_OFFSET       = 30;

        std::string                             m_windowTitle;
        TableContext<3>                         m_armorListTable;
        TableContext<3>                         m_armorCandidatesTable;
        int                                     m_armorAddPolicy    = 0;
        bool                                    m_fFilterPlayable   = false;
        bool                                    m_fShowOutfitWindow = false;
        std::array<char, MAX_FILTER_ARMOR_NAME> m_filterStringBuf;
        SosUiData                              &m_uiData;
        SosDataCoordinator                     &m_dataCoordinator;
        Armor                                  *m_selectedArmor = nullptr;
        Popup::DeleteArmorPopup                 m_DeleteArmorPopup;
        Popup::ConflictArmorPopup               m_ConflictArmorPopup;

    public:
        explicit SosGuiOutfit(SosUiData &uiData, SosDataCoordinator &dataCoordinator)
            : m_armorListTable( TableContext<3>::Create("##OutfitArmors", {"$SosGui_TableHeader_Slot", "$ARMOR", "$Delete"})),
              m_armorCandidatesTable( TableContext<3>::Create("##ArmorCandidates", {"$ARMOR", "$SosGui_TableHeader_Slot", "$Add"})),
              m_uiData(uiData), m_dataCoordinator(dataCoordinator)
        {
            m_armorListTable.Resizable().Sortable().SizingStretchProp();
            m_armorCandidatesTable.Resizable().Sortable();
        }

        auto Render(SosUiOutfit &editingOutfit) -> bool;

        /// <summary>
        /// Show thsi outfit edit window with specificy outfit-name.
        /// </summary>
        /// <param name="outfitName">be used to set window title</param>
        /// <param name="show">true, show window</param>
        void ShowWindow(const std::string &outfitName, bool show = true)
        {
            m_fShowOutfitWindow = show;
            UpdateWindowTitle(outfitName);
        }

    private:
        void UpdateWindowTitle(const std::string &outfitName);
        void RenderProperties(SosUiOutfit &editingOutfit);
        void RenderArmorList(SosUiOutfit &editingOutfit);
        void RenderEditPanel(SosUiOutfit &editingOutfit);
        auto RenderArmorSlotFilter() -> Slot;
        void RenderArmorCandidates(SosUiOutfit &editingOutfit, Slot selectedSlot);
        void RenderEditPanelPolicy(SosUiOutfit &editingOutfit);
        void RenderOutfitAddPolicyById(SosUiOutfit &editingOutfit, const bool &fFilterPlayable) const;

        void UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable, OutfitAddPolicy policy);
        void UpdateArmorCandidatesBySlot(Slot slot);
        void UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable);
        auto IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool;

        void RenderPopups(const std::string &outfitName);
    };
}