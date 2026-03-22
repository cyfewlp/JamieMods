#ifndef IMEWND_HPP
#define IMEWND_HPP

#pragma once

#include "core/State.h"
#include "ime/ITextService.h"
#include "tsf/InputMethodManager.h"
#include "ui/ImeOverlay.h"
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
static inline auto      g_tMainClassName                 = L"SimpleIME";
static constexpr size_t TRANSLATOR_DEBONCE_DELAY_SECONDS = 10LLU;

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
    void UnInitialize() noexcept;

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
    auto FocusTextService(bool focus) const -> bool;
    auto IsFocused() const -> bool;
    auto SendMessageToIme(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> LRESULT;
    auto SendNotifyMessageToIme(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> bool;

    //! Must call from IME thread.
    //! @see ImeController::CommitCandidate
    void CommitCandidate(const DWORD index) const
    {
        if (m_textService)
        {
            m_textService->CommitCandidate(index);
        }
    }

    //! Must call from IME thread.
    //! @see ImeController::SetConversionMode
    void SetConversionMode(const DWORD conversionMode) const
    {
        if (m_textService)
        {
            m_textService->SetConversionMode(conversionMode);
        }
    }

    //! Must call from IME thread.
    //! @see ImeController::ActivateLangProfile
    auto ActivateLanguageProfile(const GUID &guidProfile) const -> HRESULT;

    auto GetHWND() const -> HWND { return m_hWnd; }

    /**
     * Focus to a parent window to abort IME
     */
    void AbortIme() const;
    void Draw(Settings &settings);

private:
    static auto WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
    static auto OnNccCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) -> LRESULT;
    static void TsfMessageLoop();

    auto OnCreated(Settings &settings) -> void;

    [[nodiscard]] auto OnDestroy() -> LRESULT;

    void InitializeTextService();
    void ForwardKeyboardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const;

    DebounceTimer                   m_translatorLoadDebounceTimer{std::chrono::seconds(TRANSLATOR_DEBONCE_DELAY_SECONDS)};
    std::unique_ptr<ImeWindow>      m_imeWindow             = nullptr;
    std::unique_ptr<UI::ImeOverlay> m_imeOverlay            = nullptr;
    std::unique_ptr<ITextService>   m_textService           = nullptr;
    CComPtr<InputMethodManager>     m_inputMethodManager    = nullptr;
    HWND                            m_hWnd                  = nullptr;
    HWND                            m_hWndParent            = nullptr;
    DWORD                           m_gameThreadId          = 0;
    float                           m_uiScale               = 1.0F;
    bool                            m_fWantUpdateUiScale    = true; ///< update scale in the first frame.
    bool                            m_fFocused              = false;
    bool                            m_fEnabledTsf           = true;
    bool                            m_fJustWantCaptureMouse = false;
};
} // namespace Ime

#endif
