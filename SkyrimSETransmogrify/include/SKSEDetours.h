#pragma once

#ifndef _SKSEDetours_H
    #define _SKSEDetours_H

    #include "utils.hpp"
    #include <windows.h>

struct Logger
{
    template <typename T> static void log(int level, const T &value)
    {
        if (level > 1)
            return;
        spdlog::log(static_cast<spdlog::level::level_enum>(level), value);
    }
    template <typename... Types> static void log(int level, const char *const message, const Types &...params)
    {
        if (level > 1)
            return;
        auto fmt = std::vformat(std::string(message), std::make_format_args(params...));
        spdlog::log(static_cast<spdlog::level::level_enum>(level), fmt.c_str());
    }
};

namespace SKSEDetours
{
    typedef ATOM(WINAPI *FuncRegisterClass)(CONST WNDCLASSEXW *);

    static inline FuncRegisterClass RealRegisterClassW = NULL;
    static inline FuncRegisterClass RealRegisterClassA = NULL;
    static inline FuncRegisterClass RealRegisterClassExW = NULL;
    static inline FuncRegisterClass RealRegisterClassExA = NULL;

    ATOM WINAPI                     MyRegisterClassW(CONST WNDCLASSEXW *wndClass);
    ATOM WINAPI                     MyRegisterClassA(CONST WNDCLASSEXW *wndClass);
    ATOM WINAPI                     MyRegisterClassExA(CONST WNDCLASSEXW *wndClass);
    ATOM WINAPI                     MyRegisterClassExW(CONST WNDCLASSEXW *wndClass);

    void                            detours();
};

#endif // !_SKSEDetours_H