//
// Created by jamie on 25-1-22.
//
#include "AppConfig.h"
#include "Configs.h"
#include "spdlog/common.h"
#include <spdlog/sinks/basic_file_sink.h>

namespace
{
    void InitializeLogging(const PLUGIN_NAMESPACE::AppConfig *config)
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
        log->set_level(config->GetLogLevel());
        log->flush_on(config->GetFlushLevel());

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%-8l] [%t] [%s:%#] %v");
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    try
    {
        const auto *pConfig = PLUGIN_NAMESPACE::AppConfig::Load();

        InitializeLogging(pConfig);

        Init(skse);

        const auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
        const auto  version = plugin->GetVersion();
        LIBC_NAMESPACE::log_info("{} {} is loading...", plugin->GetName(), version.string());

        LIBC_NAMESPACE::PluginInit();

        LIBC_NAMESPACE::log_info("{} has finished loading.", plugin->GetName());
        return true;
    }
    catch (std::exception &exception)
    {
        LIBC_NAMESPACE::log_error("Fatal error, SimpleIME init fail: {}", exception.what());
    }
    catch (...)
    {
        LIBC_NAMESPACE::log_error("Fatal error. occur unknown exception.");
    }

    return false;
}