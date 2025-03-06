#ifndef IMEUI_H
#define IMEUI_H

#pragma once

#include "ImGuiThemeLoader.h"
#include "configs/AppConfig.h"
#include "ime/ITextService.h"
#include "tsf/LangProfileUtil.h"

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

            template <typename... Args>
            void PushErrorMessage(std::format_string<Args...> fmt, Args &&...args);

        private:
            void RenderSettings();
            void RenderCompWindow() const;
            void RenderCandidateWindows() const;

            static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

            AppUiConfig              m_uiConfig;
            ImGuiThemeLoader         m_uiThemeLoader{};
            LangProfileUtil         *m_langProfileUtil = nullptr;
            ITextService            *m_pTextService    = nullptr;
            ImeWnd                  *m_pImeWnd         = nullptr;
            std::vector<std::string> m_themeNames;
            uint32_t                 m_selectedTheme = 0;

            bool m_fShowToolWindow = false;
            bool m_fFollowCursor   = false;
            bool m_fShowSettings   = false;
            bool m_fPinToolWindow  = false;

            std::vector<std::string> m_errorMessages;
            int m_toolWindowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration;
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif
