#pragma once

#include "common/config.h"
#include "imgui.h"

#include <RE/T/TESObjectARMO.h>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{

class SosUiOutfit;

namespace Popup
{

struct BasicPopup
{
    ImGuiID popupId = 0;

    constexpr void Open() const
    {
        ImGui::OpenPopup(popupId);
    }

    auto preDraw(const char *nameKey, ImGuiWindowFlags flags = 0) -> bool
    {
        popupId = ImGui::GetID(nameKey);
        return ImGui::BeginPopup(nameKey, flags);
    }
};

struct ModalPopup
{
    bool    showPopup = false;
    ImGuiID popupId   = 0;

    constexpr void Open(ImGuiPopupFlags flags = 0)
    {
        if (showPopup)
        {
            return;
        }
        showPopup = true;
        ImGui::OpenPopup(popupId, flags);
    }

    auto BeginModal(const char *nameKey, ImGuiWindowFlags flags = ImGuiWindowFlags_None) -> bool;
};

using Armor = RE::TESObjectARMO;

class MessagePopup : public ModalPopup
{
public:
    MessagePopup() = default;

protected:
    static void RenderMultilineMessage(const std::string &message);
    static void RenderConfirmButtons(__out bool &confirmed);
};

struct DeleteOutfitPopup final : MessagePopup
{
    const SosUiOutfit *wanDeleteOutfit = nullptr;

    void Draw(bool &isConfirmed);
};

struct ConflictArmorPopup final : MessagePopup
{
    const Armor *conflictedArmor = nullptr;

    void Draw(bool &confirmed);
};

struct DeleteArmorPopup final : MessagePopup
{
    const Armor *wantDeleteArmor = nullptr;

    void Draw(bool &confirmed);
};

struct SlotPolicyHelp final : MessagePopup
{
    void Draw();
};

struct BatchAddArmors final : MessagePopup
{
    void Draw(__out bool &confirmed);
};
}
}