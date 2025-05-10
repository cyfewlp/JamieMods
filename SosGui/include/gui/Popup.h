#pragma once

#include "common/config.h"
#include "imgui.h"
#include "data/id.h"

#include <RE/T/TESObjectARMO.h>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
class SosUiData;

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
    bool             showPopup = true;
    bool             wantOpen  = true;
    std::string_view nameKey;
    ImGuiPopupFlags  popupFlags = 0;

    explicit ModalPopup(const std::string_view &nameKey) : nameKey(nameKey) {}

    virtual ~ModalPopup() = default;

    constexpr void Open(ImGuiPopupFlags popupFlags = ImGuiPopupFlags_None)
    {
        wantOpen         = true;
        this->popupFlags = popupFlags;
    }

    // The returen value indicates whether this popup has been closed
    virtual bool Draw(SosUiData &uiData, bool &confirmed, ImGuiWindowFlags flags = 0);

    static void RenderMultilineMessage(const std::string &message);
    static void RenderConfirmButtons(__out bool &confirmed);

protected:
    virtual void DoDraw(SosUiData &uiData, bool &confirmed) = 0;
};

using Armor = RE::TESObjectARMO;

struct DeleteOutfitPopup final : ModalPopup
{
    OutfitId    wanDeleteOutfitId;
    std::string wanDeleteOutfitName;

    DeleteOutfitPopup(const OutfitId wanDeleteOutfitId, const std::string &wanDeleteOutfitName)
        : ModalPopup("$SosGui_PopupName_ConfirmDeleteOutfit"), wanDeleteOutfitId(wanDeleteOutfitId),
          wanDeleteOutfitName(wanDeleteOutfitName) {}

protected:
    void DoDraw(SosUiData &uiData, bool &confirmed) override;
};
}
}