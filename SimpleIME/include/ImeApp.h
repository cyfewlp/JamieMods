//
// Created by jamie on 25-1-22.
//
#pragma once

#include "ImeWnd.hpp"
#include "common/hook.h"
#include "ui/Settings.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

class ImeApp
{
public:
    explicit ImeApp(Settings &settings) : m_settings(settings), m_imeWnd(m_settings) {}

    ~ImeApp()                             = default;
    ImeApp(const ImeApp &other)           = delete;
    ImeApp(ImeApp &&other)                = delete;
    ImeApp operator=(const ImeApp &other) = delete;
    ImeApp operator=(ImeApp &&other)      = delete;

    static auto GetInstance() -> ImeApp &;

    void Initialize();
    void Uninitialize();
    void OnInputLoaded();
    void Draw() const;

    constexpr auto GetGameHWND() const -> HWND
    {
        return m_hWnd;
    }

    constexpr auto GetImeWnd() -> ImeWnd &
    {
        return m_imeWnd;
    }

private:
    std::unique_ptr<Hooks::D3DInitHookData> D3DInitHook = nullptr;
    // std::unique_ptr<Hooks::D3DPresentHookData>         D3DPresentHook         = nullptr;
    // std::unique_ptr<Hooks::DispatchInputEventHookData> DispatchInputEventHook = nullptr;

    void OnD3DInit();
    void Start(const RE::BSGraphics::RendererData &renderData);

    static void InstallHooks();
    static void UninstallHooks();

    HWND             m_hWnd        = nullptr;
    HIMC             m_hIMCDefault = nullptr; // Game main window default HIMC
    Settings        &m_settings;
    ImeWnd           m_imeWnd;
    std::atomic_bool m_fInitialized = false;

    static void D3DInit();
    static void DoD3DInit();
    // static void           D3DPresent(std::uint32_t ptr);
    // static void           DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent
    // **a_events);
    static auto           MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
    static inline WNDPROC RealWndProc;
};
}
}
