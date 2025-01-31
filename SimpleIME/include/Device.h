#pragma once

#include <dinput.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

namespace SimpleIME
{
#define ERROR_CODE_DIRECTINPUT8CREATE  0x1
#define ERROR_CODE_CREATEDEVICE        0x2
#define ERROR_CODE_SETDATAFORMAT       0x3
#define ERROR_CODE_SETCOOPERATIVELEVEL 0x4
#define ERROR_CODE_ACQUIRE             0x5

    class KeyboardDevice
    {

    public:
        KeyboardDevice(HWND hWnd) noexcept(false) : m_hWnd(hWnd)
        {
            if (FAILED(DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
                                          (void **)&m_pDirectInput, NULL)))
            {
                throw SimpleIMEException("DirectInput8Create failed");
            }
            if (FAILED(m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, NULL)))
            {
                throw SimpleIMEException("CreateDevice failed");
            }
        }

        ~KeyboardDevice()
        {
            if (m_pDirectInput)
            {
                if (m_pKeyboardDevice)
                {
                    m_pKeyboardDevice->Unacquire();
                    m_pKeyboardDevice->Release();
                    m_pKeyboardDevice = NULL;
                }
                m_pDirectInput->Release();
                m_pDirectInput = NULL;
            }
        }

        BOOL SetNonExclusive() noexcept(false)
        {
            m_pKeyboardDevice->Unacquire();
            DWORD dwFlags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
            if (FAILED(m_pKeyboardDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
            {
                throw SimpleIMEException("SetCooperativeLevel failed");
            }
            if (FAILED(m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
            {
                throw SimpleIMEException("SetDataFormat failed");
            }
            if (FAILED(m_pKeyboardDevice->Acquire()))
            {
                throw SimpleIMEException("Acquire device failed");
            }
            acquired.store(true);
            return true;
        }

        BOOL GetState(__out void *buffer, __in long buffer_size) noexcept
        {
            if (!acquired.load()) return false;
            HRESULT rv;
            while (1)
            {
                m_pKeyboardDevice->Poll();
                if (SUCCEEDED(rv = m_pKeyboardDevice->GetDeviceState(buffer_size, buffer))) break;
                if (rv == DIERR_INPUTLOST || rv == DIERR_NOTACQUIRED)
                {
                    return SUCCEEDED(m_pKeyboardDevice->Acquire());
                }
                return FALSE;
            }
            return TRUE;
        }

        BOOL TryAcquire()
        {
            if (acquired.load())
            {
                logv(debug, "Try Acquire");
                return SUCCEEDED(m_pKeyboardDevice->Acquire());
            }
            return false;
        }

        void Unacquire()
        {
            m_pKeyboardDevice->Unacquire();
        }

        std::atomic<bool> acquired = false;

    private:
        HWND                 m_hWnd            = nullptr;
        LPDIRECTINPUT8       m_pDirectInput    = nullptr;
        LPDIRECTINPUTDEVICE8 m_pKeyboardDevice = nullptr;
    };
}