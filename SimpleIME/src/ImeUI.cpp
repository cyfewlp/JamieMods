//
// Created by jamie on 25-1-21.
//

#include "ImeUI.h"

#include "ImeWnd.hpp"
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

        static constexpr ImVec4 RED_COLOR = {1.0F, 0.0F, 0.0F, 1.0F};

        ImeUI::ImeUI(AppUiConfig const &uiConfig, ImeWnd *pImeWnd, ITextService *pTextService) : m_pUiConfig(uiConfig)
        {
            _tsetlocale(LC_ALL, _T(""));
            m_pTextService = pTextService;
            m_pImeWnd      = pImeWnd;
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

        void ImeUI::RenderIme() const
        {
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoDecoration;
            windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

            if (m_pTextService->HasState(ImeState::IME_DISABLED) ||
                m_pTextService->HasNoStates(ImeState::IN_CAND_CHOOSING, ImeState::IN_COMPOSING))
            {
                return;
            }

            if (m_fFollowCursor)
            {
                ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
            }
            ImGui::Begin("SimpleIME", nullptr, windowFlags);

            ImGui::SameLine();
            ImGui::BeginGroup();
            RenderCompWindow();

            ImGui::Separator();
            // render ime status window: language,
            if (m_pTextService->HasState(ImeState::IN_CAND_CHOOSING))
            {
                RenderCandidateWindows();
            }
            ImGui::EndGroup();
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

        static bool SettingsMode = false;

        void ImeUI::RenderToolWindow()
        {
            if (!m_showToolWindow)
            {
                return;
            }

            ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_showToolWindow, m_toolWindowFlags);

            RenderSettings();

            ImGui::Text("Drag");
            ImGui::SameLine();

            if (!m_errorMessages.empty())
            {
                for (const auto &errorMessage : m_errorMessages)
                {
                    ImGui::TextColored(RED_COLOR, "%s", errorMessage.c_str());
                }
            }

            if (ImGui::Button("\xf0\x9f\x93\x8c"))
            {
                m_toolWindowFlags |= ImGuiWindowFlags_NoInputs;
                m_pinToolWindow                = true;
                ImGui::GetIO().MouseDrawCursor = false;
            }
            ImGui::SameLine();

            ImGui::Checkbox("Settings", &SettingsMode);
            ImGui::SameLine();

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
                        m_pImeWnd->SendMessage_(CM_ACTIVATE_PROFILE, 0, reinterpret_cast<LPARAM>(&pair.first));
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

        void ImeUI::RenderSettings()
        {
            static bool EnableMod       = true;
            static bool CollapseVisible = false;
            CollapseVisible             = SettingsMode;

            if (SettingsMode)
            {
                if (ImGui::CollapsingHeader("Settings##Content", &CollapseVisible))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
                    if (ImGui::BeginTable("SettingsTable", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        static bool EnableModFail = false;
                        if (ImGui::Checkbox("Enable Mod", &EnableMod))
                        {
                            if (!m_pImeWnd->EnableMod(EnableMod))
                            {
                                EnableMod     = false;
                                EnableModFail = true;
                                log_debug("Unable to enable mod: {}", GetLastError());
                            }
                        }
                        ImGui::SetItemTooltip("Uncheck will disable all mod feature(Disable keyboard).");
                        if (EnableModFail)
                        {
                            ImGui::TableNextColumn();
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, .0f, .0f, 1.0f), "%s", "Failed to enable mod");
                            if (ImGui::Button("X"))
                            {
                                EnableModFail = false;
                            }
                        }

                        ImGui::TableNextColumn();
                        if (m_pTextService->HasState(ImeState::IME_DISABLED))
                        {
                            ImGui::Text("Ime Enabled %s", "\xe2\x9d\x8c"); // red  ❌
                        }
                        else
                        {
                            ImGui::Text("Ime Enabled %s", "\xe2\x9c\x85"); // green ✅
                        }
                        ImGui::TableNextRow();

                        ImGui::TableNextColumn();
                        bool const focused = m_pImeWnd->IsFocused();
                        ImGui::Text("Ime Focus: %s", focused ? "\xe2\x9c\x85" : "\xe2\x9d\x8c");
                        ImGui::SetItemTooltip("Mod must has keyboard focus to work.");

                        ImGui::TableNextColumn();
                        if (ImGui::Button("Force Focus Ime"))
                        {
                            m_pImeWnd->Focus();
                        }

                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Ime follow cursor", &m_fFollowCursor);
                        ImGui::SetItemTooltip("Ime window appear in cursor position.");
                        ImGui::EndTable();
                    }
                    ImGui::PopStyleVar();
                }
                SettingsMode = CollapseVisible;
            }
            else
            {
                ImGui::SameLine();
            }
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