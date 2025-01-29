#pragma once

#include <dinput.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

namespace SimpleIME
{
    class KeyboardDevice
    {

    public:
        ~KeyboardDevice()
        {
            if (pDirectInput)
            {
                if (pKeyboardDevice)
                {
                    pKeyboardDevice->Unacquire();
                    pKeyboardDevice->Release();
                    pKeyboardDevice = NULL;
                }
                pDirectInput->Release();
                pDirectInput = NULL;
            }
        }

        BOOL Initialize(HWND hWnd) noexcept(false)
        {
            if (FAILED(DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
                                          (void **)&pDirectInput, NULL)))
            {
                throw SimpleIMEException("DirectInput8Create failed");
            }
            if (FAILED(pDirectInput->CreateDevice(GUID_SysKeyboard, &pKeyboardDevice, NULL)))
            {
                throw SimpleIMEException("CreateDevice failed");
            }
            if (FAILED(pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
            {
                throw SimpleIMEException("SetDataFormat failed");
            }

            if (FAILED(pKeyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            {
                throw SimpleIMEException("SetCooperativeLevel failed");
            }
            return true;
        }

        BOOL Acquire(__out void *buffer, __in long buffer_size) noexcept
        {
            if (FAILED(pKeyboardDevice->Acquire())) return FALSE;
            HRESULT rv;
            while (1)
            {
                pKeyboardDevice->Poll();
                if (SUCCEEDED(rv = pKeyboardDevice->GetDeviceState(buffer_size, buffer))) break;
                if (rv != DIERR_INPUTLOST || rv != DIERR_NOTACQUIRED) return FALSE;
                if (FAILED(pKeyboardDevice->Acquire())) return FALSE;
            }
            pKeyboardDevice->Unacquire();
            return TRUE;
        }

    private:
        // 创建DirectInput对象
        LPDIRECTINPUT8       pDirectInput;
        LPDIRECTINPUTDEVICE8 pKeyboardDevice = nullptr;
    };
}