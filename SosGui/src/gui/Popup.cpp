#include "gui/Popup.h"

#include "Translation.h"
#include "common/config.h"
#include "imgui.h"
#include "data/SosUiOutfit.h"
#include "util/ImGuiUtil.h"

#include <ranges>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{

inline auto Popup::ModalPopup::BeginModal(const char *nameKey, const ImGuiWindowFlags flags) -> bool
{
    const auto name = Translation::Translate(nameKey);
    popupId = ImGui::GetID(name.c_str());
    return ImGui::BeginPopupModal(name.c_str(), &showPopup, flags);
}

void Popup::MessagePopup::RenderMultilineMessage(const std::string &message)
{
    constexpr auto delim = "\\n"sv;
    const auto *viewport = ImGui::GetMainViewport();
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

void Popup::MessagePopup::RenderConfirmButtons(__out bool &confirmed)
{
    const float contentWidth = ImGui::GetContentRegionAvail().x;
    const auto quarterWidth = contentWidth * 0.25F;
    ImGui::SetCursorPosX(quarterWidth * 0.5F);
    if (ImGuiUtil::Button("$Yes", ImVec2(quarterWidth, 0.0F)))
    {
        confirmed = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();

    ImGui::SetCursorPosX(quarterWidth * 2.5F);
    if (ImGuiUtil::Button("$No", ImVec2(quarterWidth, 0.0F)))
    {
        ImGui::CloseCurrentPopup();
    }
}

void Popup::DeleteOutfitPopup::Draw(bool &isConfirmed)
{
    isConfirmed = false;
    if (!BeginModal("$SosGui_PopupName_ConfirmDeleteOutfit", ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    if (wanDeleteOutfit == nullptr)
    {
        ImGui::EndPopup();
        return;
    }

    const auto message = Translation::Translate("$SkyOutSys_Confirm_Delete_Text{}", true, wanDeleteOutfit->GetName());
    ImGui::Text("%s", message.c_str());
    RenderConfirmButtons(isConfirmed);
    ImGui::EndPopup();
}

void Popup::ConflictArmorPopup::Draw(bool &confirmed)
{
    confirmed = false;
    if (!BeginModal("$SosGui_Confirm_ArmorConflict", ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    if (conflictedArmor == nullptr)
    {
        ImGui::EndPopup();
        return;
    }
    const auto message = Translation::Translate("$SkyOutSys_Confirm_BodySlotConflict_Text");
    RenderMultilineMessage(message);
    RenderConfirmButtons(confirmed);
    ImGui::EndPopup();
}

void Popup::DeleteArmorPopup::Draw(bool &confirmed)
{
    confirmed = false;
    if (!BeginModal("$SosGui_Confirm_ArmorDelete", ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    if (wantDeleteArmor == nullptr)
    {
        ImGui::EndPopup();
        return;
    }
    const auto message = Translation::Translate("$SkyOutSys_Confirm_RemoveArmor_Text{}", true,
                                                wantDeleteArmor->GetName());
    ImGui::Text("%s", message.c_str());
    RenderConfirmButtons(confirmed);
    ImGui::EndPopup();
}

void Popup::SlotPolicyHelp::Draw()
{
    const auto mainViewPort = ImGui::GetMainViewport();
    const auto maxSize = ImVec2(mainViewPort->WorkSize.x * 0.5, mainViewPort->WorkSize.y * 0.5);
    ImGui::SetNextWindowSize(maxSize, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(maxSize.x * 0.5, maxSize.y * 0.5), ImGuiCond_Appearing);
    if (!BeginModal("$SkyOutSys_OEdit_SlotPolicyHelp"))
    {
        return;
    }
    auto DrawEscaped = [](std::string &&str) {
        size_t pos = 0;
        while ((pos = str.find("\\n", pos)) != std::string::npos)
        {
            str.replace(pos, 2, "\n");
            pos += 1;
        }
        ImGui::TextWrapped("%s", str.c_str());
    };
    DrawEscaped(Translation::Translate("$SkyOutSys_OEdit_SlotPolicy_HelpText1"));
    DrawEscaped(Translation::Translate("$SkyOutSys_OEdit_SlotPolicy_HelpText2"));
    DrawEscaped(Translation::Translate("$SkyOutSys_OEdit_SlotPolicy_HelpText3"));
    ImGui::EndPopup();
}

void Popup::BatchAddArmors::Draw(__out bool &confirmed)
{
    confirmed = false;
    if (!BeginModal("$SosGui_BatchAddArmors", ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    const auto message = Translation::Translate("$SosGui_BatchAddArmors_Message");
    RenderMultilineMessage(message);
    RenderConfirmButtons(confirmed);
    ImGui::EndPopup();
}

}