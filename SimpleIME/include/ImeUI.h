#ifndef IMEUI_H
#define IMEUI_H

#pragma once

#include "common/imgui/ThemesLoader.h"
#include "common/utils.h"
#include "configs/AppConfig.h"
#include "core/State.h"
#include "core/Translation.h"
#include "ime/ITextService.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImeWnd;

constexpr auto MAX_ERROR_MESSAGE_COUNT = 10;

class ImeUI
{
    using State = Core::State;

public:
    explicit ImeUI(AppUiConfig const &uiConfig, ImeWnd *pImeWnd, ITextService *pTextService)
        : m_uiConfig(uiConfig), m_pImeWnd(pImeWnd), m_pTextService(pTextService),
          m_themesLoader(CommonUtils::GetInterfaceFile(ImGuiUtil::ThemesLoader::DEFAULT_THEME_FILE))
    {
    }

    ~ImeUI();
    ImeUI(const ImeUI &other)                = delete;
    ImeUI(ImeUI &&other) noexcept            = delete;
    ImeUI &operator=(const ImeUI &other)     = delete;
    ImeUI &operator=(ImeUI &&other) noexcept = delete;

    bool Initialize(LangProfileUtil *pLangProfileUtil);
    void Draw(const Settings &settings);
    void RenderToolWindow(Settings &settings);
    void ShowToolWindow();
    void ApplyUiSettings(Settings &settings);

private:
    auto UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) const -> bool;
    auto UpdateImeWindowPosByCaret(ImVec2 &windowPos) const -> bool;
    // Calculate ime window size by candidate string and composition string.
    // Because we need to place the IME window in screen according to the IME window size;
    void CalculateWindowSize();

    void DrawInputMethodsCombo() const;
    void DrawSettings(Settings &settings);
    void DrawModConfig(Settings &settings);
    void DrawFeatures(Settings &settings);
    void DrawSettingsContent(Settings &settings);
    void DrawSettingsFocusManage(Settings &settings) const;
    void DrawStates() const;
    void DrawWindowPosUpdatePolicy(Settings &settings);
    void DrawCompWindow(const Settings &settings) const;
    void DrawCandidateWindows() const;
    auto Translate(const char *label) const -> const char *;

    static bool DrawCombo(const char *label, const std::vector<std::string> &values, std::string &selected);

    static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");

    AppUiConfig              m_uiConfig;
    ImeWnd                  *m_pImeWnd         = nullptr;
    ITextService            *m_pTextService    = nullptr;
    LangProfileUtil         *m_langProfileUtil = nullptr;
    Translation              m_translation;
    std::vector<std::string> m_translateLanguages;
    ImVec2                   m_imeWindowSize = ImVec2(0, 0);
    ImGuiUtil::ThemesLoader  m_themesLoader;

    bool m_fShowToolWindow = false;
    bool m_fPinToolWindow  = false;

    std::vector<std::string> m_errorMessages;
};
} // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif
