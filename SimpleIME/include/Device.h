#ifndef DEVICE_H
#define DEVICE_H

#pragma once

#include <atomic>
#include <dinput.h>
#include <minwindef.h>
#include <stdexcept>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        class KeyboardDevice
        {

        public:
            KeyboardDevice(KeyboardDevice &&other)                          = delete;
            auto operator=(KeyboardDevice &&other) -> KeyboardDevice &      = delete;

            KeyboardDevice(const KeyboardDevice &other)                     = delete;
            auto operator=(const KeyboardDevice &other) -> KeyboardDevice & = delete;

            explicit KeyboardDevice(HWND hWnd) noexcept(false) : m_hWnd(hWnd)
            {
                if (FAILED(DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
                                              reinterpret_cast<void **>(&m_pDirectInput), nullptr)))
                {
                    throw std::runtime_error("DirectInput8Create failed");
                }
                if (FAILED(m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, nullptr)))
                {
                    throw std::runtime_error("CreateDevice failed");
                }
            }

            ~KeyboardDevice()
            {
                if (m_pDirectInput != nullptr)
                {
                    if (m_pKeyboardDevice != nullptr)
                    {
                        m_pKeyboardDevice->Unacquire();
                        m_pKeyboardDevice->Release();
                        m_pKeyboardDevice = nullptr;
                    }
                    m_pDirectInput->Release();
                    m_pDirectInput = nullptr;
                }
            }

            // will throw runtime_error about failed reason
            void Acquire() noexcept(false)
            {
                acquired.store(false);
                m_pKeyboardDevice->Unacquire();
                const DWORD dwFlags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
                if (FAILED(m_pKeyboardDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
                {
                    throw std::runtime_error("SetCooperativeLevel failed");
                }
                if (FAILED(m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
                {
                    throw std::runtime_error("SetDataFormat failed");
                }
                if (FAILED(m_pKeyboardDevice->Acquire()))
                {
                    throw std::runtime_error("Acquire device failed");
                }
                acquired.store(true);
            }

            // just log failed reason
            void TryAcquire() noexcept {
                acquired.store(false);
                m_pKeyboardDevice->Unacquire();
                const DWORD dwFlags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
                if (FAILED(m_pKeyboardDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
                {
                    log_error("keyboard can't set to DISCL_NONEXCLUSIVE");
                    return;
                }
                if (FAILED(m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
                {
                    log_error("keyboard can't SetDataFormat");
                    return;
                }
                if (FAILED(m_pKeyboardDevice->Acquire()))
                {
                    log_warn("Can't acquire keyboard, is game window frozen?");
                    return;
                }
                acquired.store(true);
            }

            auto GetState() noexcept -> bool
            {
                if (!IsAcquired())
                {
                    return false;
                }
                HRESULT hresult = TRUE;
                m_pKeyboardDevice->Poll();
                hresult = m_pKeyboardDevice->GetDeviceState(sizeof(state), reinterpret_cast<LPVOID>(state.data()));
                if (hresult == DIERR_INPUTLOST || hresult == DIERR_NOTACQUIRED)
                {
                    TryAcquire();
                    hresult = m_pKeyboardDevice->GetDeviceState(sizeof(state), reinterpret_cast<LPVOID>(state.data()));
                }
                return SUCCEEDED(hresult);
            }

            auto IsKeyPress(const uint8_t keyCode) -> bool
            {
                return (state[keyCode] & 0x80) > 0;
            }

            auto IsAcquired() noexcept -> bool
            {
                return acquired.load();
            }

        private:
            HWND                  m_hWnd            = nullptr;
            LPDIRECTINPUT8        m_pDirectInput    = nullptr;
            LPDIRECTINPUTDEVICE8  m_pKeyboardDevice = nullptr;
            std::atomic<bool>     acquired          = false;
            std::array<char, 256> state{0};
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif