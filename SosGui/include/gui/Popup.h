#pragma once

#include "common/config.h"
#include "imgui.h"

#include <string>

namespace LIBC_NAMESPACE_DECL
{
    namespace Popup
    {
        struct PopupContext
        {
            std::string popupNameKey;
            ImGuiID     popupId       = 0;
            bool        m_fConfirmed  = false;
            bool        m_fLastClosed = true;

            constexpr void Open(ImGuiPopupFlags flags = 0) { ImGui::OpenPopup(popupId, flags); }

            [[nodiscard]] constexpr auto IsLastClosed() const -> bool { return m_fLastClosed; }

        protected:
            void RenderConfirmButtons();
        };

        using Armor = RE::TESObjectARMO;

        class MessagePopup : public PopupContext
        {
            ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

        public:
            MessagePopup(const std::string_view messageKey) : messageKey(messageKey) {}

            ~MessagePopup() = default;

        protected:
            std::string_view messageKey;
            auto             PreRender() -> bool;
            static void      RenderMultilineMessage(const std::string &message);
        };

        struct DeleteOutfitPopup : MessagePopup
        {
            DeleteOutfitPopup() : MessagePopup("$SkyOutSys_Confirm_Delete_Text{}")
            {
                popupNameKey = "$SosGui_PopupName_ConfirmDeleteOutfit";
            }

            // return true if this popup just closed;
            auto Render(const std::string &outfitName, bool &isConfirmed) -> bool;
        };

        struct ConflictArmorPopup : MessagePopup
        {
            ConflictArmorPopup() : MessagePopup("$SkyOutSys_Confirm_BodySlotConflict_Text")
            {
                popupNameKey = "$SosGui_Confirm_ArmorConflict";
            }

            auto Render(Armor *armor) -> bool;
        };

        struct DeleteArmorPopup : MessagePopup
        {
            DeleteArmorPopup() : MessagePopup("$SkyOutSys_Confirm_RemoveArmor_Text{}")
            {
                popupNameKey = "$SosGui_Confirm_ArmorDelete";
            }

            auto Render(Armor *armor) -> bool;
        };

        struct SlotPolicyHelp : MessagePopup
        {
            SlotPolicyHelp() : MessagePopup("$SosGui_SlotPolicy_HelpText{}")
            {
                popupNameKey = "$SkyOutSys_OEdit_SlotPolicyHelp";
            }

            void Render();
        };

        struct BatchAddArmors : MessagePopup
        {
            BatchAddArmors() : MessagePopup("$SosGui_BatchAddArmors_Message")
            {
                popupNameKey = "$SosGui_BatchAddArmors";
            }

            auto Render() -> bool;
        };
    }
}