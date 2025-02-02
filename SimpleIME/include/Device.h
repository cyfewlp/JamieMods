#ifndef _SIMPLE_IME_DEVICE_
#define _SIMPLE_IME_DEVICE_

#pragma once

#include <atomic>
#include <dinput.h>
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
            KeyboardDevice(KeyboardDevice &&other)                 = delete;
            KeyboardDevice &operator=(KeyboardDevice &&other)      = delete;

            KeyboardDevice(const KeyboardDevice &other)            = delete;
            KeyboardDevice &operator=(const KeyboardDevice &other) = delete;

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

            void SetNonExclusive() noexcept(false)
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

            auto GetState(__in LPVOID buffer, __in DWORD buffer_size) noexcept -> bool
            {
                if (!acquired.load())
                {
                    return false;
                }
                HRESULT hresult = TRUE;
                while (true)
                {
                    m_pKeyboardDevice->Poll();
                    if (SUCCEEDED(hresult = m_pKeyboardDevice->GetDeviceState(buffer_size, buffer)))
                    {
                        break;
                    }
                    if (hresult == DIERR_INPUTLOST || hresult == DIERR_NOTACQUIRED)
                    {
                        return SUCCEEDED(m_pKeyboardDevice->Acquire());
                    }
                }
                return SUCCEEDED(hresult);
            }

            std::atomic<bool> acquired = false;

        private:
            HWND                 m_hWnd            = nullptr;
            LPDIRECTINPUT8       m_pDirectInput    = nullptr;
            LPDIRECTINPUTDEVICE8 m_pKeyboardDevice = nullptr;
        };
    }
}
#endif