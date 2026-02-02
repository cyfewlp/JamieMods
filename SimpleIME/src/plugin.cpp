//
// Created by jamie on 25-1-22.
//
#include "common/common.h"
#include "common/log.h"
#include "configs/ConfigSerializer.h"

#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace SksePlugin
{
void InitializeLogging(const spdlog::level::level_enum logLevel, const spdlog::level::level_enum flushLevel)
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
    log->set_level(logLevel);
    log->flush_on(flushLevel);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%-8l] [%t] [%s:%#] %v");
}

bool PluginLoad(const SKSE::LoadInterface *skse)
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

int ErrorHandler(unsigned int code, _EXCEPTION_POINTERS *)
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
