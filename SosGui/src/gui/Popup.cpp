#include "gui/Popup.h"

#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "imgui.h"
#include "data/SosUiOutfit.h"
#include "util/ImGuiUtil.h"

#include <cstdint>
#include <format>
#include <ranges>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
auto Popup::MessagePopup::PreRender(const char*nameKey) -> bool
{
    auto name = Translation::Translate(nameKey);
    popupId = ImGui::GetID(name.c_str());
    return ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
}

void Popup::MessagePopup::RenderMultilineMessage(const std::string &message)
{
    constexpr auto delim = "\\n"sv;
    auto *viewport = ImGui::GetMainViewport();
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
        auto textSize = ImGui::CalcTextSize(line.data());
        if (contentWidth > textSize.x)
        {
            ImGui::SetCursorPosX((contentWidth - textSize.x) * 0.5F);
        }
        ImGui::Text("%s", line.data());
    }
    ImGui::PopTextWrapPos();
}

void Popup::PopupContext::RenderConfirmButtons(__out bool &confirmed)
{
    float contentWidth = ImGui::GetContentRegionAvail().x;
    auto quarterWidth = contentWidth * 0.25F;
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

void Popup::DeleteOutfitPopup::Render(bool &isConfirmed)
{
    isConfirmed = false;
    if (!PreRender("$SosGui_PopupName_ConfirmDeleteOutfit"))
    {
        return;
    }
    if (wanDeleteOutfit == nullptr)
    {
        ImGui::EndPopup();
        return;
    }

    auto message = Translation::Translate("$SkyOutSys_Confirm_Delete_Text{}", true, wanDeleteOutfit->GetName());

    ImGuiUtil::TextScale(message.c_str(), HintFontSize());
    RenderConfirmButtons(isConfirmed);

    ImGui::EndPopup();
}

void Popup::ConflictArmorPopup::Render(bool &confirmed)
{
    confirmed = false;
    if (!PreRender("$SosGui_Confirm_ArmorConflict"))
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

void Popup::DeleteArmorPopup::Render(bool &confirmed)
{
    confirmed = false;
    if (!PreRender("$SosGui_Confirm_ArmorDelete"))
    {
        return;
    }
    if (wantDeleteArmor == nullptr)
    {
        ImGui::EndPopup();
        return;
    }
    const auto message = Translation::Translate("$SkyOutSys_Confirm_RemoveArmor_Text{}", true, wantDeleteArmor->GetName());

    ImGuiUtil::TextScale(message.c_str(), HintFontSize());
    RenderConfirmButtons(confirmed);
    ImGui::EndPopup();
}

void Popup::SlotPolicyHelp::Render()
{
    if (!PreRender("$SkyOutSys_OEdit_SlotPolicyHelp"))
    {
        return;
    }
    constexpr const char *templateStr = "$SkyOutSys_OEdit_SlotPolicy_HelpText{}";
    static uint8_t index = 1;
    const auto message = Translation::Translate(std::format(templateStr, index).c_str());
    RenderMultilineMessage(message);
    ImGui::BeginDisabled(index <= 1);
    if (ImGuiUtil::Button("$SosGui_Table_PrevPage"))
    {
        index--;
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(index > 3);
    ImGui::SameLine();
    if (ImGuiUtil::Button("$SosGui_Table_NextPage"))
    {
        index++;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGuiUtil::Button("$SosGui_Button_Close"))
    {
        index = 1;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Popup::BatchAddArmors::Render(__out bool &confirmed)
{
    confirmed = false;
    if (!PreRender("$SosGui_BatchAddArmors"))
    {
        return;
    }
    const auto message = Translation::Translate("$SosGui_BatchAddArmors_Message");
    RenderMultilineMessage(message);
    RenderConfirmButtons(confirmed);
    ImGui::EndPopup();
}

}