#pragma once

#include "ImGuiUtil.h"
#include "SosUiData.h"
#include "Translation.h"
#include "common/config.h"
#include "data/SosUiOutfit.h"
#include "gui/SosDataCoordinator.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <functional>
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
        ImGuiUtil::ImTable<3>                   m_armorListTable;
        ImGuiUtil::ImTable<3>                   m_armorCandidatesTable;
        int                                     m_armorAddPolicy  = 0;
        bool                                    m_fFilterPlayable = false;
        std::array<char, MAX_FILTER_ARMOR_NAME> m_filterStringBuf;
        SosUiData                              &m_uiData;
        SosDataCoordinator                     &m_dataCoordinator;

        bool m_fShowOutfitWindow = false;

        class ConfirmPopup
        {
            std::string name;
            ImGuiID     id;
            Armor      *data      = nullptr;
            bool        isOpen    = false;
            bool        isConfirm = false;

            std::function<void(Armor *)> onConfirm = nullptr;

        public:
            explicit ConfirmPopup(const std::string_view &nameKey) : id(0)
            {
                name = Translation::Translate(nameKey.data());
            }

            void Open(Armor *data, const std::function<void(Armor *)> &callback);
            void Close();
            void Render(const std::string_view &message);

            constexpr auto GetData() const -> Armor *
            {
                return data;
            }
        };

        ConfirmPopup m_armorConflictConfirmPopup;
        ConfirmPopup m_armorRemoveConfirmPopup;

    public:
        explicit SosGuiOutfit(SosUiData &uiData, SosDataCoordinator &dataCoordinator)
            : m_uiData(uiData), m_dataCoordinator(dataCoordinator),
              m_armorConflictConfirmPopup("$SosGui_Confirm_ArmorConflict"),
              m_armorRemoveConfirmPopup("$SosGui_Confirm_ArmorDelete")
        {
            m_armorListTable.name = "##OutfitArmors";
            m_armorListTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
            m_armorListTable.flags |= ImGuiTableFlags_SizingStretchProp;
            m_armorListTable.headersRow = {Translation::Translate("$SosGui_TableHeader_Slot"),
                                           Translation::Translate("$ARMOR"), Translation::Translate("$Delete")};
            m_armorCandidatesTable.name = "##ArmorCandidates";
            m_armorCandidatesTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
            m_armorCandidatesTable.headersRow = {
                Translation::Translate("$ARMOR"),
                Translation::Translate("$SosGui_TableHeader_Slot"),
                Translation::Translate("$Add"),
            };
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
    };
}