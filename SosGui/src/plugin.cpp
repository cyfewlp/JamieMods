
#include "common.h"
#include "log.h"

#include <exception>
#include <excpt.h>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>

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

bool PluginLoad(const SKSE::LoadInterface *skse)
{
    try
    {
        const auto *plugin = SKSE::PluginDeclaration::GetSingleton();

        InitializeLogging({spdlog::level::debug, spdlog::level::trace});

        Init(skse);

        const auto version = plugin->GetVersion();
        logger::info("{} {} is loading...", plugin->GetName(), version.string());

        Initialize();

        logger::info("{} has finished loading.", plugin->GetName());
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
