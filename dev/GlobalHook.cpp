#include "spdlog/spdlog.h"
#include <filesystem>
#include <iostream>
#include <optional>
#include <spdlog/sinks/basic_file_sink.h>
#include <windows.h>

std::atomic<bool> initializedLog      = false;
HHOOK             g_hHook             = NULL;
HMODULE           g_hMod              = NULL;
char              g_szTitle[MAX_PATH] = {0};

static void       InitializeLogging()
{
    try
    {
        auto new_logger = spdlog::basic_logger_mt(
            "new_default_logger",
            "C:\\Users\\jamie\\Documents\\My Games\\Skyrim Special Edition\\SKSE\\new-default-log.txt", true);
        spdlog::set_default_logger(new_logger);
        new_logger->set_level(spdlog::level::debug);
        spdlog::info("new logger log message");
        initializedLog = true;
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    spdlog::log(spdlog::level::info, "LowLevelKeyboardProc {}", wParam);
    return (CallNextHookEx(g_hHook, nCode, wParam, lParam));
}

extern "C" __declspec(dllexport) BOOL SetKbdHook()
{
    BOOL    fOk     = FALSE;
    LRESULT lResult = 0;

    g_hHook         = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, g_hMod, 0);
    if (NULL == g_hHook)
    {
        return (FALSE);
    }

    return (TRUE);
}

extern "C" __declspec(dllexport) VOID UnhookKbdHook()
{
    if (NULL != g_hHook)
    {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (!initializedLog.load())
    {
        InitializeLogging();
    }
    spdlog::log(spdlog::level::info, "DllMain {}", ul_reason_for_call);
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: {
            g_hMod = hModule;
            break;
        }
        case DLL_PROCESS_DETACH: {
            break;
        }
        default: {
            break;
        }
    }

    return (TRUE);
}