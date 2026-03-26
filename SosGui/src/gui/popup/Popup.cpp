#include "gui/popup/Popup.h"

#include "Translation.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"

#include <ranges>
#include <string>

namespace SosGui
{
bool Popup::ModalPopup::Draw(SosUiData &uiData, bool &confirmed, ImGuiWindowFlags flags)
{
    confirmed       = false;
    const auto name = Translation::Translate(nameKey.data());
    if (wantOpen)
    {
        wantOpen = false;
        ImGui::OpenPopup(name.c_str(), popupFlags);
    }
    if (!ImGui::BeginPopupModal(name.c_str(), &showPopup, flags))
    {
        return showPopup;
    }
    DoDraw(uiData, confirmed);
    ImGui::EndPopup();
    return showPopup;
}

void Popup::ModalPopup::RenderMultilineMessage(const std::string &message)
{
    constexpr auto delim    = "\\n"sv;
    const auto    *viewport = ImGui::GetMainViewport();
    ImGui::PushTextWrapPos(ImGui::GetCursorScreenPos().x + viewport->WorkSize.x * 0.5F);
    const auto &contentWidth = ImGui::GetContentRegionAvail().x;
    for (const auto &lineView : std::views::split(message, delim))
    {
        auto lineStrView = std::string_view(lineView);
        if (lineStrView.empty())
        {
            continue;
        }
        auto line = std::string(lineStrView);
        if (const auto textSize = ImGui::CalcTextSize(line.data()); contentWidth > textSize.x)
        {
            ImGui::SetCursorPosX((contentWidth - textSize.x) * 0.5F);
        }
        ImGui::Text("%s", line.data());
    }
    ImGui::PopTextWrapPos();
}

void Popup::ModalPopup::RenderConfirmButtons(__out bool &confirmed)
{
    const float contentWidth = ImGui::GetContentRegionAvail().x;
    const auto  quarterWidth = contentWidth * 0.25F;
    ImGui::SetCursorPosX(quarterWidth * 0.5F);
    if (ImGuiUtil::Button("$Yes", ImVec2(quarterWidth, 0.0F)))
    {
        ConfirmAndClose(confirmed);
    }
    ImGui::SameLine();

    ImGui::SetCursorPosX(quarterWidth * 2.5F);
    if (ImGuiUtil::Button("$No", ImVec2(quarterWidth, 0.0F)))
    {
        ImGui::CloseCurrentPopup();
    }
}

void Popup::DeleteOutfitPopup::DoDraw(SosUiData &, bool &confirmed)
{
    const auto message = Translation::Translate("$SkyOutSys_Confirm_Delete_Text{}", true, wanDeleteOutfitName);
    ImGui::Text("%s", message.c_str());
    RenderConfirmButtons(confirmed);
}
} // namespace SosGui
