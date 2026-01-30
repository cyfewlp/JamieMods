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
    struct State
    {
        enum class StateKey : std::uint8_t
        {
            UNINITIALIZED,
            INITIALIZING,
            INITIALIZED,
            INITIALIZE_FAILED,
            DORMANCY
        };

    private:
        std::atomic<StateKey> m_stateKey = StateKey::UNINITIALIZED;

    public:
        constexpr auto GetStateKetText(StateKey stateKey) const -> std::string
        {
            switch (stateKey)
            {
                case StateKey::INITIALIZED:
                    return "Initialized";
                case StateKey::INITIALIZING:
                    return "Initializing";
                case StateKey::INITIALIZE_FAILED:
                    return "Initialization failed";
                case StateKey::UNINITIALIZED:
                    return "Uninitialized";
                case StateKey::DORMANCY:
                    return "Dormancy";
            }
            return "Unknown state";
        }

        constexpr auto GetStateKetText() const -> std::string
        {
            return GetStateKetText(m_stateKey);
        }

        void SetState(const StateKey stateKey)
        {
            if (stateKey <= m_stateKey)
            {
                log_error(
                    "The state cannot be rolled back from [{}] to [{}]!",
                    GetStateKetText(m_stateKey),
                    GetStateKetText(stateKey)
                );
                return;
            }
            m_stateKey.exchange(stateKey);
        }

        constexpr auto IsUnInitialized() const
        {
            return m_stateKey == StateKey::UNINITIALIZED;
        }

        constexpr auto IsInitializing() const
        {
            return m_stateKey == StateKey::INITIALIZING;
        }

        constexpr auto IsInitializeFailed() const
        {
            return m_stateKey == StateKey::INITIALIZE_FAILED;
        }

        constexpr auto IsInitialized() const
        {
            return m_stateKey == StateKey::INITIALIZED;
        }
    };

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

    constexpr auto GetState() const -> const State &
    {
        return m_state;
    }

private:
    std::unique_ptr<Hooks::D3DInitHookData> D3DInitHook = nullptr;
    // std::unique_ptr<Hooks::D3DPresentHookData>         D3DPresentHook         = nullptr;
    // std::unique_ptr<Hooks::DispatchInputEventHookData> DispatchInputEventHook = nullptr;

    void OnD3DInit();
    void Start(const RE::BSGraphics::RendererData &renderData);

    static void InstallHooks();
    static void UninstallHooks();

    inline void LogAlreadyInitialized() const;

    HWND      m_hWnd        = nullptr;
    HIMC      m_hIMCDefault = nullptr; // Game main window default HIMC
    Settings &m_settings;
    ImeWnd    m_imeWnd;
    State     m_state;

    static void D3DInit();
    void        DoD3DInit();
    // static void           D3DPresent(std::uint32_t ptr);
    // static void           DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent
    // **a_events);
    static auto           MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
    static inline WNDPROC RealWndProc;
};
}
}
