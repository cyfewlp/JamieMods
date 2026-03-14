//
// Created by jamie on 25-1-22.
//
#include "common.h"
#include "configs/ConfigSerializer.h"
#include "log.h"

#include <spdlog/sinks/basic_file_sink.h>

namespace SksePlugin
{
void InitializeLogging(SpdLogSettings settings)
{
    auto path = SKSE::log::log_directory();
    if (!path)
    {
        SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
    }
    *path /= SKSE::PluginDeclaration::GetSingleton()->GetName();
    *path += L".log";

    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    auto log  = std::make_shared<spdlog::logger>(std::string("global log"), std::move(sink));
    log->set_level(settings.level);
    log->flush_on(settings.flushLevel);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%-8l] [%t] [%s:%#] %v");
}

auto PluginLoad(const SKSE::LoadInterface *skse) -> bool
{
    try
    {
        Init(skse, false);
        Initialize();
        return true;
    }
    catch (std::exception &exception)
    {
        logger::error("Fatal error, SimpleIME init fail: {}", exception.what());
        logger::LogStacktrace();
    }
    catch (...)
    {
        logger::error("Fatal error. occur unknown exception.");
        logger::LogStacktrace();
    }
    return false;
}

auto ErrorHandler(unsigned int code, _EXCEPTION_POINTERS *) -> int
{
    logger::critical("System exception (code {}) raised during plugin initialization.", code);
    logger::LogStacktrace();
    return EXCEPTION_CONTINUE_SEARCH;
}
} // namespace SksePlugin

SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    __try
    {
        return SksePlugin::PluginLoad(skse);
    }
    __except (SksePlugin::ErrorHandler(GetExceptionCode(), GetExceptionInformation()))
    {
    }
    return false;
}

namespace Ime::Global
{
auto g_hModule = HMODULE();
}

extern "C" auto APIENTRY DllMain(const HMODULE hModule, const DWORD ul_reason, LPVOID /*unused*/) -> BOOL
{
    switch (ul_reason)
    {
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_ATTACH:
            Ime::Global::g_hModule = hModule;
            break;
        case DLL_PROCESS_DETACH:
            spdlog::shutdown();
            break;
        default:;
    }
    return TRUE;
}
