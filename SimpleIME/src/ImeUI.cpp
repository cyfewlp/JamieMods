﻿//
// Created by jamie on 25-1-21.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ImeUI.h"

#include "ImGuiThemeLoader.h"
#include "ImeWnd.hpp"
#include "common/WCharUtils.h"
#include "common/log.h"
#include "context.h"
#include "ime/ImeManager.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"
#include "ui/ImeUIWidgets.h"

#include <clocale>
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
            SetTranslate();
            return true;
        }

        void ImeUI::SetTranslate()
        {
            if (!m_translation.GetTranslateLanguages(m_uiConfig.TranslationDir(), m_translateLanguages) ||
                !m_translation.UseLanguage(m_uiConfig.DefaultLanguage().c_str()))
            {
                log_warn("Failed load translation languages.");
            }
            else
            {
                const auto defaultLang = std::ranges::find(m_translateLanguages, m_uiConfig.DefaultLanguage());
                size_t     index       = std::distance(m_translateLanguages.begin(), defaultLang);
                index                  = std::min(index, m_translateLanguages.size() - 1);
                m_imeUIWidgets.SetUInt32Var("$Languages", index);
            }
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
            m_imeUIWidgets.SetUInt32Var("$Themes", std::distance(m_themeNames.begin(), findIt));
            auto &style = ImGui::GetStyle();
            m_uiThemeLoader.LoadTheme(defaultTheme, style);
        }

        void ImeUI::RenderIme() const
        {
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize;
            windowFlags |= ImGuiWindowFlags_NoDecoration;
            windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;

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

            m_translation.UseSection("Tool Window");
            ImGui::Begin(TOOL_WINDOW_NAME.data(), &m_fShowToolWindow, m_toolWindowFlags);

            RenderSettings();

            m_translation.UseSection("Tool Window");

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
            m_imeUIWidgets.Checkbox("$Settings", m_fShowSettings);

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
            auto *state                      = State::GetInstance();

            if (!m_fShowSettings)
            {
                return;
            }

            m_imeUIWidgets.Begin("$Settings", &isSettingsWindowOpen, ImGuiWindowFlags_NoNav);
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
            do
            {
                bool fEnableMod = State::GetInstance()->IsModEnabled();
                m_imeUIWidgets.Checkbox("$Enable_Mod", fEnableMod, [](bool EnableMod) {
                    return Ime::ImeManagerComposer::GetInstance()->NotifyEnableMod(EnableMod);
                });

                m_imeUIWidgets.ComboApply("$Languages", m_translateLanguages, [this](const std::string &lang) {
                    m_translation.UseLanguage(lang.c_str());
                    return true;
                });

                if (!fEnableMod)
                {
                    break;
                }

                enum FocusManage
                {
                    Permanent,
                    Temporary
                };

                // Focus Manage widget
                ImGui::SeparatorText(m_translation.Get("$Focus_Manage"));
                static int focusManager = Permanent;
                int        currentFocusManage = focusManager;
                m_imeUIWidgets.RadioButton("$Focus_Manage_Permanent", &focusManager, Permanent);
                ImGui::SameLine();
                m_imeUIWidgets.RadioButton("$Focus_Manage_Temporary", &focusManager, Temporary);
                if (focusManager != currentFocusManage)
                {
                    if (focusManager == Permanent)
                    {
                        ImeManagerComposer::GetInstance()->UsePermanentFocusImeManager();
                    }
                    else
                    {
                        ImeManagerComposer::GetInstance()->UseTemporaryFocusImeManager();
                    }
                    ImeManagerComposer::GetInstance()->SyncImeState();
                }

                ImGui::SeparatorText(m_translation.Get("$States"));

                m_imeUIWidgets.StateWidget("$Ime_Enabled", state->NotHas(State::IME_DISABLED));

                m_imeUIWidgets.StateWidget("$Ime_Focus", m_pImeWnd->IsFocused());

                ImGui::SameLine();
                m_imeUIWidgets.Button("$Force_Focus_Ime", []() {
                    // ReSharper disable once CppExpressionWithoutSideEffects
                    ImeManagerComposer::GetInstance()->ForceFocusIme();
                });

                m_imeUIWidgets.Checkbox("$Ime_Follow_Ime", m_fFollowCursor);

                ImGui::SameLine();
                bool fKeepImeOpen = Context::GetInstance()->KeepImeOpen();
                m_imeUIWidgets.Checkbox("$Keep_Ime_Open", fKeepImeOpen, [](const bool keepImeOpen) {
                    Context::GetInstance()->SetKeepImeOpen(keepImeOpen);
                    return Ime::ImeManagerComposer::GetInstance()->SyncImeState();
                });
                ImGui::Value("TSF Focus", State::GetInstance()->Has(State::TSF_FOCUS));
            } while (false);
            ImGui::PopStyleVar();

            m_imeUIWidgets.ComboApply("$Themes", m_themeNames, [this](const std::string &name) {
                return m_uiThemeLoader.LoadTheme(name, ImGui::GetStyle());
            });
            ImGui::End();
            m_fShowSettings = isSettingsWindowOpen;
        }

        void ImeUI::RenderCompWindow() const
        {
            const ImVec4 highLightText = ImGui::GetStyle().Colors[ImGuiCol_TextLink];
            const auto  &editorText    = m_pTextService->GetTextEditor().GetText();
            const auto   str           = WCharUtils::ToString(editorText);
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