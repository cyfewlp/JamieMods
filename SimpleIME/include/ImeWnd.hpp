#ifndef IMEWND_HPP
#define IMEWND_HPP

#pragma once

#include "ImeUI.h"
#include "core/State.h"
#include "ime/ITextService.h"
#include "tsf/LangProfileUtil.h"

#include <atlcomcli.h>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImmImeHandler;
static inline auto g_tMainClassName = L"SimpleIME";

class ImeWnd
{
    using State = Core::State;

public:
    ImeWnd(Settings &settings);
    ~ImeWnd();

    ImeWnd(ImeWnd &&a_imeWnd)                 = delete;
    ImeWnd(const ImeWnd &a_imeWnd)            = delete;
    ImeWnd &operator=(ImeWnd &&a_imeWnd)      = delete;
    ImeWnd &operator=(const ImeWnd &a_imeWnd) = delete;

    void Initialize() noexcept(false);
    void UnInitialize() const noexcept;

    /**
     * Work on standalone thread and run own message loop.
     * Mainly avoid other plugins that init COM
     * with COINIT_MULTITHREADED(crash logger) to affect our TSF code.
     *
     * @param hWndParent Main window (game window)
     * @param pSettings
     */
    void Start(HWND hWndParent, Settings *pSettings);

    auto Focus() const -> void;
    auto SetTsfFocus(bool focus) const -> bool;
    auto IsFocused() const -> bool;
    auto SendMessageToIme(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> bool;
    auto SendNotifyMessageToIme(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> bool;
    auto GetImeThreadId() const -> DWORD;

    constexpr auto GetHWND() const -> HWND
    {
        return m_hWnd;
    }

    /**
     * Focus to a parent window to abort IME
     */
    void AbortIme() const;
    void DrawIme(Settings &settings) const;
    void ShowToolWindow() const;

    bool IsShowingToolWindow() const
    {
        return m_pImeUi->IsShowingToolWindow();
    }

    bool IsPinedToolWindow() const
    {
        return m_pImeUi->IsPinedToolWindow();
    }

    void ApplyUiSettings(Settings *pSettings) const;

private:
    static auto WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
    static auto GetThis(HWND hWnd) -> ImeWnd *;
    static auto OnNccCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) -> LRESULT;
    static void OnCompositionResult(const std::wstring &compositionString);
    static void TsfMessageLoop(MSG msg);

    void        OnStart(Settings *pSettings);
    static auto OnCreate() -> LRESULT;
    auto        OnDestroy() const -> LRESULT;
    void        InitializeTextService();
    void        ForwardKeyboardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const;

    Settings                     &m_settings;
    std::unique_ptr<ImeUI>        m_pImeUi       = nullptr;
    std::unique_ptr<ITextService> m_pTextService = nullptr;
    CComPtr<LangProfileUtil>      m_pLangProfileUtil;
    HWND                          m_hWnd       = nullptr;
    HWND                          m_hWndParent = nullptr;
    WNDCLASSEXW                   wc{};
    bool                          m_fFocused = false;
};
} // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
