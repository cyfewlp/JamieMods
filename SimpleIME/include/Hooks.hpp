#ifndef HOOKS_HPP
#define HOOKS_HPP

#pragma once

#include "common/common.h"

#include <cstdint>
#include <unknwn.h>
#include <windows.h>

enum : std::uint8_t
{
    GET_MSG_PROC = 0,
    NUMHOOKS
};

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        using FuncRegisterClass      = ATOM (*)(const WNDCLASSA *);
        using FuncDirectInput8Create = HRESULT (*)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
        static inline FuncRegisterClass      RealRegisterClassExA   = nullptr;
        static inline FuncDirectInput8Create RealDirectInput8Create = nullptr;

        // Windows Hook
        using MYHOOKDATA = struct _MYHOOKDATA
        {
            int      nType;
            HOOKPROC hkprc;
            HHOOK    hhook;
        } __attribute__((packed));

        LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam);
        void             InstallRegisterClassHook();
        void             InstallDirectInPutHook();
        void             InstallWindowsHooks();

        auto WINAPI      MyRegisterClassExA(const WNDCLASSA *wndClass) -> ATOM;
        auto WINAPI      MyDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut,
                                              LPUNKNOWN punkOuter) -> HRESULT;

    }; // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
