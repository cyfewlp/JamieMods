#include "SKSEDetours.h"
#include "detours/detours.h"
#include "utils.hpp"
#include <windows.h>

inline void doLogDetours(const WNDCLASSEXW *wndClass, const char *funcName)
{
    auto ws = utils::WideStrToStrUTF8(wndClass->lpszClassName);
    Logger::log(1, "detours {}, classname: {}", funcName, ws.c_str());
}

ATOM __stdcall SKSEDetours::MyRegisterClassExW(const WNDCLASSEXW *wndClass)
{
    doLogDetours(wndClass, __func__);
    return RealRegisterClassExW(wndClass);
}

ATOM __stdcall SKSEDetours::MyRegisterClassW(const WNDCLASSEXW *wndClass)
{
    doLogDetours(wndClass, __func__);
    return RealRegisterClassW(wndClass);
}

ATOM __stdcall SKSEDetours::MyRegisterClassA(const WNDCLASSEXW *wndClass)
{
    doLogDetours(wndClass, __func__);
    HMODULE hModule = NULL; // 获取函数指针所在模块的句柄
    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)wndClass->lpfnWndProc, &hModule))
    {
        TCHAR moduleName[MAX_PATH];
        // 获取模块文件名
        if (GetModuleFileName(hModule, moduleName, MAX_PATH))
        {
            std::wcout << L"Function is in module: " << moduleName << std::endl;
            LOG(debug, "lpfnWndProc is in module:  {}", utils::WideStrToStrUTF8(moduleName).c_str());
        }
        else
        {
            LOG(err, "Failed to get module file name");
        }
    }
    else
    {
        LOG(err, "Failed to get module handle");
    }
    return RealRegisterClassA(wndClass);
}

ATOM __stdcall SKSEDetours::MyRegisterClassExA(const WNDCLASSEXW *wndClass)
{
    doLogDetours(wndClass, __func__);
    return RealRegisterClassExA(wndClass);
}

static void doDetours(LPCSTR pszModule, LPCSTR pszFunction, std::function<void(PVOID)> operation)
{
    PVOID pVoid = DetourFindFunction(pszModule, pszFunction);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    operation(pVoid);
    LONG error = DetourTransactionCommit();
    if (error == NO_ERROR)
    {
        Logger::log(1, "{}: Detoured {}.", pszModule, pszFunction);
    }
    else
    {
        Logger::log(1, "{}: Error Detouring {}.", pszModule, pszFunction);
    }
}

void SKSEDetours::detours()
{
    doDetours("User32.dll", "RegisterClassExW", [](PVOID pVoid) -> void {
        RealRegisterClassExW = reinterpret_cast<FuncRegisterClass>(pVoid);
        DetourAttach(&(PVOID &)RealRegisterClassExW, MyRegisterClassExW);
    });
    doDetours("User32.dll", "RegisterClassExA", [](PVOID pVoid) -> void {
        RealRegisterClassExA = reinterpret_cast<FuncRegisterClass>(pVoid);
        DetourAttach(&(PVOID &)RealRegisterClassExA, MyRegisterClassExA);
    });
    doDetours("User32.dll", "RegisterClassW", [](PVOID pVoid) -> void {
        RealRegisterClassW = reinterpret_cast<FuncRegisterClass>(pVoid);
        DetourAttach(&(PVOID &)RealRegisterClassW, MyRegisterClassW);
    });
    doDetours("User32.dll", "RegisterClassA", [](PVOID pVoid) -> void {
        RealRegisterClassA = reinterpret_cast<FuncRegisterClass>(pVoid);
        DetourAttach(&(PVOID &)RealRegisterClassA, MyRegisterClassA);
    });
}
