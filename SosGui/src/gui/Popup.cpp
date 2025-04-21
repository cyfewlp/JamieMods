#include "gui/Popup.h"

#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"

#include <cstdint>
#include <format>
#include <ranges>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    auto Popup::MessagePopup::PreRender() -> bool
    {
        m_fConfirmed  = false;
        m_fLastClosed = false;
        auto name     = Translation::Translate(popupNameKey);
        popupId       = ImGui::GetID(name.c_str());
        return ImGui::BeginPopupModal(name.c_str(), nullptr, flags);
    }

    void Popup::MessagePopup::RenderMultilineMessage(const std::string &message)
    {
        constexpr auto delim    = "\\n"sv;
        auto          *viewport = ImGui::GetMainViewport();
        ImGui::PushTextWrapPos(ImGui::GetCursorScreenPos().x + viewport->WorkSize.x * 0.5F);
        const auto &contentWidth = ImGui::GetContentRegionAvail().x;
        for (const auto &lineView : std::views::split(message, delim))
        {
            auto lineStrView = std::string_view(lineView);
            if (lineStrView.empty())
            {
                continue;
            }
            auto line     = std::string(lineStrView);
            auto textSize = ImGui::CalcTextSize(line.data());
            if (contentWidth > textSize.x)
            {
                ImGui::SetCursorPosX((contentWidth - textSize.x) * 0.5F);
            }
            ImGui::Text("%s", line.data());
        }
        ImGui::PopTextWrapPos();
    }

    void Popup::PopupContext::RenderConfirmButtons()
    {
        float contentWidth = ImGui::GetContentRegionAvail().x;
        auto  quarterWidth = contentWidth * 0.25F;
        ImGui::SetCursorPosX(quarterWidth * 0.5F);
        if (ImGuiUtil::Button("$Yes", ImVec2(quarterWidth, 0.0F)))
        {
            m_fConfirmed  = true;
            m_fLastClosed = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        ImGui::SetCursorPosX(quarterWidth * 2.5F);
        if (ImGuiUtil::Button("$No", ImVec2(quarterWidth, 0.0F)))
        {
            m_fLastClosed = true;
            ImGui::CloseCurrentPopup();
        }
    }

    auto Popup::DeleteOutfitPopup::Render(const std::string &outfitName, bool &isConfirmed) -> bool
    {
        isConfirmed = false;
        if (!PreRender())
        {
            return false;
        }

        auto message = Translation::Translate(messageKey.data(), true, outfitName);

        ImGuiUtil::TextScale(message.c_str(), HintFontSize());
        RenderConfirmButtons();

        ImGui::EndPopup();
        isConfirmed = m_fConfirmed;
        return m_fLastClosed;
    }

    auto Popup::ConflictArmorPopup::Render(Armor *armor) -> bool
    {
        if (!PreRender())
        {
            return m_fConfirmed;
        }
        if (armor == nullptr)
        {
            ImGui::EndPopup();
            return m_fConfirmed;
        }
        const auto message = Translation::Translate(messageKey.data());
        RenderMultilineMessage(message);
        RenderConfirmButtons();
        ImGui::EndPopup();
        return m_fConfirmed;
    }

    auto Popup::DeleteArmorPopup::Render(Armor *armor) -> bool
    {
        if (!PreRender())
        {
            return m_fConfirmed;
        }
        if (armor == nullptr)
        {
            ImGui::EndPopup();
            return m_fConfirmed;
        }
        const auto message = Translation::Translate(messageKey.data(), true, armor->GetName());

        ImGuiUtil::TextScale(message.c_str(), HintFontSize());
        RenderConfirmButtons();
        ImGui::EndPopup();
        return m_fConfirmed;
    }

    void Popup::SlotPolicyHelp::Render()
    {
        if (!PreRender())
        {
            return;
        }
        constexpr const char *templateStr = "$SkyOutSys_OEdit_SlotPolicy_HelpText{}";
        static uint8_t        index       = 1;
        const auto            message     = Translation::Translate(std::format(templateStr, index).c_str());
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

    auto Popup::BatchAddArmors::Render() -> bool
    {
        if (!PreRender())
        {
            return m_fConfirmed;
        }
        const auto message = Translation::Translate(messageKey.data());
        RenderMultilineMessage(message);
        RenderConfirmButtons();
        ImGui::EndPopup();
        return m_fConfirmed;
    }

}