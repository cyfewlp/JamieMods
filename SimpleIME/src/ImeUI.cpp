//
// Created by jamie on 25-1-21.
//

#include "ImeUI.h"
#include "common/WCharUtils.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "tsf/LangProfileUtil.h"
#include "tsf/TextStore.h"

#include "imgui.h"
#include <clocale>
#include <cstdint>
#include <tchar.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        ImeUI::ImeUI(AppUiConfig const &uiConfig, ITextService *pTextService) : m_pUiConfig(uiConfig)
        {
            _tsetlocale(LC_ALL, _T(""));
            m_pTextService = pTextService;
        }

        ImeUI::~ImeUI()
        {
            if (m_pTextService != nullptr)
            {
                m_langProfileUtil->Release();
            }
        }

        bool ImeUI::Initialize(LangProfileUtil *pLangProfileUtil)
        {
            log_debug("Initializing ImeUI...");
            m_langProfileUtil = pLangProfileUtil;
            m_langProfileUtil->AddRef();
            if (!m_langProfileUtil->LoadAllLangProfiles())
            {
                log_error("Failed load lang profiles");
                return false;
            }
            if (!m_langProfileUtil->LoadActiveIme())
            {
                log_error("Failed load active ime");
                return false;
            }
            return true;
        }

        void ImeUI::SetHWND(const HWND hWnd)
        {
            m_hWndIme = hWnd;
        }

        void ImeUI::RenderIme()
        {
            RenderToolWindow();

            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoDecoration;
            windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

            if (m_pTextService->HasState(ImeState::IME_DISABLED) ||
                m_pTextService->HasNoStates(ImeState::IN_CAND_CHOOSING, ImeState::IN_COMPOSING))
            {
                return;
            }

            ImGui::Begin("SimpleIME", nullptr, windowFlags);

            ImGui::SameLine();
            RenderCompWindow();

            ImGui::Separator();
            // render ime status window: language,
            if (m_pTextService->HasState(ImeState::IN_CAND_CHOOSING))
            {
                RenderCandidateWindows();
            }
            ImGui::End();
        }

        void ImeUI::ShowToolWindow()
        {
            m_toolWindowFlags &= ~ImGuiWindowFlags_NoInputs;
            if (m_pinToolWindow)
            {
                m_pinToolWindow                = false;
                ImGui::GetIO().MouseDrawCursor = true;
                ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
            }
            else
            {
                m_showToolWindow               = !m_showToolWindow;
                ImGui::GetIO().MouseDrawCursor = m_showToolWindow;
                if (m_showToolWindow)
                {
                    ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
                }
            }
        }

        void ImeUI::RenderToolWindow()
        {
            if (!m_showToolWindow)
            {
                return;
            }

            ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_showToolWindow, m_toolWindowFlags);

            if (!m_errorMessages.empty())
            {
                for (const auto &errorMessage : m_errorMessages)
                {
                    static constexpr ImVec4 redColor = {1.0F, 0.0F, 0.0F, 1.0F};
                    ImGui::TextColored(redColor, "%s", errorMessage.c_str());
                }
            }

            if (!m_pinToolWindow)
            {
                ImGui::Text("Drag");
                ImGui::SameLine();
            }
            if (ImGui::Button("\xf0\x9f\x93\x8c"))
            {
                m_toolWindowFlags |= ImGuiWindowFlags_NoInputs;
                m_pinToolWindow                = true;
                ImGui::GetIO().MouseDrawCursor = false;
            }

            auto        activatedGuid     = m_langProfileUtil->GetActivatedLangProfile();
            auto        installedProfiles = m_langProfileUtil->GetLangProfiles();
            auto       &profile           = installedProfiles[activatedGuid];
            const char *previewImeName    = profile.desc.c_str();
            ImGui::SameLine();
            if (ImGui::BeginCombo("###InstalledIME", previewImeName))
            {
                uint32_t idx = 0;
                for (const std::pair<GUID, LangProfile> pair : installedProfiles)
                {
                    auto       langProfile = pair.second;
                    bool const isSelected  = langProfile.guidProfile == activatedGuid;
                    auto       label       = std::format("{}##{}", langProfile.desc, idx);
                    if (ImGui::Selectable(label.c_str()))
                    {
                        // SendMessageW()  // wait handle message
                        // PostMessageW(); // not wait
                        SendMessageW(m_hWndIme, CM_ACTIVATE_PROFILE, 0, reinterpret_cast<LPARAM>(&pair.first));
                        activatedGuid = m_langProfileUtil->GetActivatedLangProfile();
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                    idx++;
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (m_pTextService->HasState(ImeState::IN_ALPHANUMERIC))
            {
                ImGui::Text("ENG");
                ImGui::SameLine();
            }
            if (!m_pinToolWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                ShowToolWindow();
            }
            ImGui::End();
        }

        void ImeUI::RenderCompWindow() const
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_pUiConfig.HighlightTextColor());
            const auto &editorText = m_pTextService->GetTextEditor().GetText();
            const auto  str        = WCharUtils::ToString(editorText);
            ImGui::Text("%s", str.c_str());
            ImGui::PopStyleColor(1);
        }

        void ImeUI::RenderCandidateWindows() const
        {
            const auto &candidateUi = m_pTextService->GetCandidateUi();
            if (const auto candidateList = candidateUi.CandidateList(); candidateList.size() > 0)
            {
                DWORD index = 0;
                for (const auto &candidate : candidateList)
                {
                    if (index == candidateUi.Selection())
                    {
                        ImGui::TextColored(ImColor(m_pUiConfig.HighlightTextColor()), "%s", candidate.c_str());
                    }
                    else
                    {
                        ImGui::Text("%s", candidate.c_str());
                    }
                    ImGui::SameLine();
                    index++;
                }
            }
        }

        template <typename... Args>
        void ImeUI::PushErrorMessage(std::format_string<Args...> fmt, Args &&...args)
        {
            if (m_errorMessages.size() >= MAX_ERROR_MESSAGE_COUNT)
            {
                auto deleteCount = m_errorMessages.size() - MAX_ERROR_MESSAGE_COUNT + 1;
                m_errorMessages.erase(m_errorMessages.begin(), m_errorMessages.begin() + deleteCount);
            }
            m_errorMessages.push_back(std::format(fmt, std::forward<Args>(args)...));
        }

    } // namespace  SimpleIME
} // namespace LIBC_NAMESPACE_DECL