//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImGuiThemeLoader.h"
#include "ImeWnd.hpp"
#include "common/WCharUtils.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "context.h"
#include "hooks/ScaleformHook.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
#include "tsf/TextStore.h"
#include "ui/ImeUIWidgets.h"

#include <clocale>
#include <cstdint>
#include <tchar.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {

        static constexpr ImVec4 RED_COLOR = {1.0F, 0.0F, 0.0F, 1.0F};

        ImeUI::ImeUI(AppUiConfig const &uiConfig, ImeWnd *pImeWnd, ITextService *pTextService) : m_uiConfig(uiConfig)
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

        void ImeUI::SetTheme()
        {
            if (m_uiConfig.UseClassicTheme())
            {
                ImGui::StyleColorsDark();
                return;
            }

            if (!m_uiThemeLoader.GetAllThemeNames(m_uiConfig.ThemeDirectory(), m_themeNames))
            {
                log_warn("Failed get theme names, fallback to ImGui default theme.");
                ImGui::StyleColorsDark();
                return;
            }

            auto      &defaultTheme = m_uiConfig.DefaultTheme();
            const auto findIt       = std::ranges::find(m_themeNames, defaultTheme);
            if (findIt == m_themeNames.end())
            {
                log_warn("Can't find default theme, fallback to ImGui default theme.");
                ImGui::StyleColorsDark();
                return;
            }
            m_selectedTheme = std::distance(m_themeNames.begin(), findIt);
            auto &style     = ImGui::GetStyle();
            m_uiThemeLoader.LoadTheme(defaultTheme, style);
        }

        void ImeUI::RenderIme() const
        {
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoDecoration;
            windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

            ;
            if (State::GetInstance()->Has(State::IME_DISABLED) ||
                State::GetInstance()->NotHas(State::IN_CAND_CHOOSING, State::IN_COMPOSING))
            {
                return;
            }

            if (m_fFollowCursor)
            {
                if (auto *cursor = RE::MenuCursor::GetSingleton(); cursor != nullptr)
                {
                    ImGui::SetNextWindowPos({cursor->cursorPosX, cursor->cursorPosY}, ImGuiCond_Appearing);
                }
            }
            ImGui::Begin("SimpleIME", nullptr, windowFlags);

            ImGui::SameLine();
            ImGui::BeginGroup();
            RenderCompWindow();

            ImGui::Separator();
            // render ime status window: language,
            if (State::GetInstance()->Has(State::IN_CAND_CHOOSING))
            {
                RenderCandidateWindows();
            }
            ImGui::EndGroup();
            ImGui::End();
        }

        void ImeUI::ShowToolWindow()
        {
            m_toolWindowFlags &= ~ImGuiWindowFlags_NoInputs;
            if (m_fPinToolWindow)
            {
                m_fPinToolWindow               = false;
                ImGui::GetIO().MouseDrawCursor = true;
                ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
            }
            else
            {
                m_fShowToolWindow              = !m_fShowToolWindow;
                ImGui::GetIO().MouseDrawCursor = m_fShowToolWindow;
                if (m_fShowToolWindow)
                {
                    ImGui::SetWindowFocus(TOOL_WINDOW_NAME.data());
                }
            }
        }

        void ImeUI::RenderToolWindow()
        {
            if (!m_fShowToolWindow)
            {
                return;
            }

            ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_fShowToolWindow, m_toolWindowFlags);

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
                m_fPinToolWindow               = true;
                ImGui::GetIO().MouseDrawCursor = false;
            }

            ImGui::SameLine();
            ImGui::Checkbox("Settings", &m_fShowSettings);

            ImGui::SameLine();
            ImeUIWidgets::RenderInputMethodChooseWidget(m_langProfileUtil, m_pImeWnd);

            ImGui::SameLine();
            if (State::GetInstance()->Has(State::IN_ALPHANUMERIC))
            {
                ImGui::Text("ENG");
                ImGui::SameLine();
            }
            ImGui::End();
        }

        void ImeUI::RenderSettings()
        {
            static bool isSettingsWindowOpen = false;
            isSettingsWindowOpen             = m_fShowSettings;

            if (!m_fShowSettings)
            {
                ImGui::SameLine();
                return;
            }
            ImGui::Begin("Settings", &isSettingsWindowOpen, ImGuiWindowFlags_None);
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
            if (ImGui::BeginTable("SettingsTable", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImeUIWidgets::RenderEnableModWidget([this](bool enable) { return m_pImeWnd->EnableMod(enable); });

                ImGui::TableNextColumn();
                ImeUIWidgets::RenderImeStateWidget(State::GetInstance()->Has(State::IME_DISABLED));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImeUIWidgets::RenderImeFocusStateWidget(m_pImeWnd->IsFocused());

                ImGui::TableNextColumn();
                if (ImGui::Button("Force Focus Ime"))
                {
                    m_pImeWnd->Focus();
                }

                ImGui::TableNextColumn();
                ImGui::Checkbox("Ime follow cursor", &m_fFollowCursor);
                ImGui::SetItemTooltip("Ime window appear in cursor position.");

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImeUIWidgets::RenderKeepImeOpenWidget([this](const bool keepImeOpen) {
                    if (keepImeOpen)
                    {
                        return m_pImeWnd->SendMessage(CM_IME_ENABLE, TRUE, 0);
                    }
                    const auto count = Hooks::ScaleformAllowTextInput::TextEntryCount();
                    return m_pImeWnd->SendMessage(CM_IME_ENABLE, count == 0 ? FALSE : TRUE, 0);
                });
                ImGui::EndTable();
            }
            ImGui::PopStyleVar();

            ImeUIWidgets::RenderThemeChooseWidget(m_themeNames, m_selectedTheme, [this](const std::string &name) {
                return m_uiThemeLoader.LoadTheme(name, ImGui::GetStyle());
            });
            ImGui::End();
            m_fShowSettings = isSettingsWindowOpen;
        }

        void ImeUI::RenderCompWindow() const
        {
            ImVec4      highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
            const auto &editorText    = m_pTextService->GetTextEditor().GetText();
            const auto  str           = WCharUtils::ToString(editorText);
            ImGui::TextColored(highLightText, "%s", str.c_str());
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
                        ImVec4 highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
                        ImGui::TextColored(highLightText, "%s", candidate.c_str());
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