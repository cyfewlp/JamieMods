#ifndef IMEUI_H
#define IMEUI_H

#pragma once

#include "ImGuiThemeLoader.h"
#include "configs/AppConfig.h"
#include "core/State.h"
#include "core/Translation.h"
#include "ime/ITextService.h"
#include "tsf/LangProfileUtil.h"
#include "ui/ErrorNotifier.h"
#include "ui/ImeUIWidgets.h"

#include "imgui.h"
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImeWnd;

        constexpr auto MAX_ERROR_MESSAGE_COUNT = 10;

        class ImeUI
        {
            using State = Ime::Core::State;

        public:
            explicit ImeUI(AppUiConfig const &uiConfig, ImeWnd *pImeWnd, ITextService *pTextService);
            ~ImeUI();
            ImeUI(const ImeUI &other)                = delete;
            ImeUI(ImeUI &&other) noexcept            = delete;
            ImeUI &operator=(const ImeUI &other)     = delete;
            ImeUI &operator=(ImeUI &&other) noexcept = delete;

            bool Initialize(LangProfileUtil *pLangProfileUtil);
            void SetTheme();
            void RenderIme() const;
            void RenderToolWindow();
            void ShowToolWindow();
            void ApplyUiSettings(const SettingsConfig &settingsConfig);
            void SyncUiSettings(SettingsConfig &settingsConfig);

        private:
            static auto UpdateImeWindowPos(bool showIme, bool &updated) -> void;
            static auto UpdateImeWindowPosByCaret() -> bool;

            void RenderSettings();
            void RenderSettingsState() const;
            void RenderSettingsFocusManage();
            void RenderSettingsImePosUpdatePolicy();
            void RenderCompWindow() const;
            void RenderCandidateWindows() const;

            static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

            AppUiConfig              m_uiConfig;
            LangProfileUtil         *m_langProfileUtil = nullptr;
            ITextService            *m_pTextService    = nullptr;
            ImeWnd                  *m_pImeWnd         = nullptr;
            ImGuiThemeLoader         m_uiThemeLoader{};
            std::vector<std::string> m_themeNames;
            Translation              m_translation;
            ImeUIWidgets             m_imeUIWidgets{&m_translation};
            std::vector<std::string> m_translateLanguages;
            ErrorNotifier            m_errorNotifier{};

            bool m_fShowToolWindow = false;
            bool m_fShowSettings   = false;
            bool m_fPinToolWindow  = false;

            std::vector<std::string> m_errorMessages;
            int                      m_toolWindowFlags =
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration;
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif
