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

        BOOL Initialize(HWND hWnd) throw(int)
        {
            if (FAILED(DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
                                          (void **)&pDirectInput, NULL)))
            {
                throw ERROR_CODE_DIRECTINPUT8CREATE;
            }
            if (FAILED(pDirectInput->CreateDevice(GUID_SysKeyboard, &pKeyboardDevice, NULL)))
            {
                throw ERROR_CODE_CREATEDEVICE;
            }
            if (FAILED(pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
            {
                throw ERROR_CODE_SETDATAFORMAT;
            }

            if (FAILED(pKeyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
            {
                throw ERROR_CODE_SETCOOPERATIVELEVEL;
            }
            return true;
        }

        BOOL Acquire(__out void *buffer, __in long buffer_size) throw(int)
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