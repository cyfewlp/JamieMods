#pragma once

#include "data/id.h"
#include "imgui.h"

#include <RE/T/TESObjectARMO.h>
#include <string>

namespace SosGui
{
class SosUiData;
class SosUiOutfit;

namespace Popup
{
struct BasicPopup
{
    ImGuiPopupFlags popupFlags = 0;
};

struct ModalPopup
{
    ImGuiPopupFlags  popupFlags = 0;
    bool             showPopup  = true;
    bool             wantOpen   = true;
    std::string_view nameKey;

    explicit ModalPopup(const std::string_view &nameKey) : nameKey(nameKey) {}

    virtual ~ModalPopup() = default;

    constexpr void Open(ImGuiPopupFlags a_popupFlags = ImGuiPopupFlags_None)
    {
        wantOpen         = true;
        this->popupFlags = a_popupFlags;
    }

    // The returen value indicates whether this popup has been closed
    virtual bool Draw(SosUiData &uiData, bool &confirmed, ImGuiWindowFlags flags = 0);

    static void RenderMultilineMessage(std::string_view message);
    static void RenderConfirmButtons(__out bool &confirmed);

protected:
    virtual void DoDraw(SosUiData &uiData, bool &confirmed) = 0;

    static void ConfirmAndClose(bool &confirmed)
    {
        confirmed = true;
        ImGui::CloseCurrentPopup();
    }
};

using Armor = RE::TESObjectARMO;

struct DeleteOutfitPopup final : ModalPopup
{
    OutfitId    wanDeleteOutfitId;
    std::string wanDeleteOutfitName;

    DeleteOutfitPopup(const OutfitId wanDeleteOutfitId, const std::string &wanDeleteOutfitName)
        : ModalPopup("$SosGui_PopupName_ConfirmDeleteOutfit"), wanDeleteOutfitId(wanDeleteOutfitId), wanDeleteOutfitName(wanDeleteOutfitName)
    {
    }

protected:
    void DoDraw(SosUiData &uiData, bool &confirmed) override;
};

void DrawSettingsPopup(std::string_view name);
void DrawAboutPopup(std::string_view name);

} // namespace Popup
} // namespace SosGui
