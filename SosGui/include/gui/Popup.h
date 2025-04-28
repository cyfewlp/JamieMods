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
struct PopupContext
{
    ImGuiID popupId = 0;

    constexpr void Open(ImGuiPopupFlags flags = 0) const
    {
        ImGui::OpenPopup(popupId, flags);
    }

protected:
    static void RenderConfirmButtons(__out bool &confirmed);
};

using Armor = RE::TESObjectARMO;

class MessagePopup : public PopupContext
{
public:
    MessagePopup() = default;
    ~MessagePopup() = default;

protected:
    auto PreRender(const char *nameKey) -> bool;
    static void RenderMultilineMessage(const std::string &message);
};

struct DeleteOutfitPopup : MessagePopup
{
    const SosUiOutfit *wanDeleteOutfit = nullptr;

    // return true if this popup just closed;
    void Render(bool &isConfirmed);
};

struct ConflictArmorPopup : MessagePopup
{
    Armor *conflictedArmor = nullptr;

    void Render(bool &confirmed);
};

struct DeleteArmorPopup : MessagePopup
{
    Armor *wantDeleteArmor = nullptr;

    void Render(bool &confirmed);
};

struct SlotPolicyHelp : MessagePopup
{
    void Render();
};

struct BatchAddArmors : MessagePopup
{
    void Render(__out bool &confirmed);
};
}
}