#ifndef IMEWND_HPP
#define IMEWND_HPP

#pragma once

#include "ImeUI.h"
#include "core/State.h"
#include "ime/ITextService.h"
#include "tsf/InputMethodManager.h"
#include "ui/ImeWindow.h"

#include <atlcomcli.h>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace ImGuiEx::M3
{
class M3Styles;
}

namespace Ime
{
class ImmImeHandler;
static inline auto g_tMainClassName = L"SimpleIME";

class ImeWnd
{
    using State = Core::State;

public:
    ImeWnd() = default;
    ~ImeWnd();

    ImeWnd(ImeWnd &&a_imeWnd)                 = delete;
    ImeWnd(const ImeWnd &a_imeWnd)            = delete;
    ImeWnd &operator=(ImeWnd &&a_imeWnd)      = delete;
    ImeWnd &operator=(const ImeWnd &a_imeWnd) = delete;

    void Initialize(bool enableTsf) noexcept(false);
    void UnInitialize() const noexcept;

    /**
     * Work on standalone thread and run own message loop.
     * Mainly avoid other plugins that init COM
     * with COINIT_MULTITHREADED(crash logger) to affect our TSF code.
     *
     * @param hWndParent Main window (game window)
     * @param settings @Settings
     */
    void CreateHost(HWND hWndParent, Settings &settings);
    void Run() const;

    auto Focus() const -> void;
    auto SetTsfFocus(bool focus) const -> bool;
    auto IsFocused() const -> bool;
    auto SendMessageToIme(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> LRESULT;
    auto SendNotifyMessageToIme(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> bool;

    void CommitCandidate(const DWORD index) const
    {
        if (m_pTextService)
        {
            m_pTextService->CommitCandidate(index);
        }
    }

    auto ActivateLanguageProfile(const GUID &guidProfile) const -> HRESULT;

    auto GetHWND() const -> HWND { return m_hWnd; }

    /**
     * Focus to a parent window to abort IME
     */
    void AbortIme() const;
    void DrawIme(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles);
    void ToggleToolWindow();

    bool IsShowingToolWindow() const { return m_pImeUi->IsShowingToolWindow(); }

    bool IsPinedToolWindow() const { return m_pImeUi->IsPinedToolWindow(); }

    void ApplyUiSettings(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles) const;

private:
    static auto WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
    static auto OnNccCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) -> LRESULT;
    static void OnCompositionResult(const std::wstring &compositionString);
    static void TsfMessageLoop();

    auto OnCreated(Settings &settings) -> void;
    auto OnDestroy() const -> LRESULT;
    void InitializeTextService();
    void ForwardKeyboardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const;

    std::unique_ptr<ImeWindow>    m_pImeWindow   = nullptr;
    std::unique_ptr<ImeUI>        m_pImeUi       = nullptr;
    std::unique_ptr<ITextService> m_pTextService = nullptr;
    CComPtr<InputMethodManager>   m_pInputMethodManager;
    HWND                          m_hWnd                  = nullptr;
    HWND                          m_hWndParent            = nullptr;
    float                         m_dpiScale              = 1.0F;
    bool                          m_fFocused              = false;
    bool                          m_fEnabledTsf           = true;
    bool                          m_fWantToggleToolWindow = false;
    bool                          m_fWantUpdateUiScale    = false;
};
} // namespace Ime

#endif
