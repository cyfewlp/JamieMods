#ifndef HOOKS_HPP
#define HOOKS_HPP

#pragma once

#include "common/config.h"
#include "common/log.h"

#include <cstdint>
#include <windows.h>
#include <vadefs.h>

enum : std::uint8_t
{
    GET_MSG_PROC = 0,
    NUMHOOKS
};

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        using FuncRegisterClass                              = ATOM (*)(const WNDCLASSA *);
        static inline FuncRegisterClass RealRegisterClassExA = nullptr;

        // Windows Hook
        using MYHOOKDATA = struct _MYHOOKDATA
        {
            int      nType;
            HOOKPROC hkprc;
            HHOOK    hhook;
        } ATTR_PACKED;

        LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam);
        void             InstallRegisterClassHook();
        void             InstallWindowsHooks();

        auto WINAPI MyRegisterClassExA(const WNDCLASSA *wndClass) -> ATOM;

    }; // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
