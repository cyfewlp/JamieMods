//
// Created by jamie on 25-1-22.
//
#include "Configs.h"
#include "ImeApp.h"

static void InitializeLogging(LIBC_NAMESPACE::SimpleIME::FontConfig *config)
{
    auto path = SKSE::log::log_directory();
    if (!path)
    {
        SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
    }
    *path /= SKSE::PluginDeclaration::GetSingleton()->GetName();
    *path += L".log";

    std::shared_ptr<spdlog::logger> log;

    auto                            sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    log                                  = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
    log->set_level(config->GetLogLevel());
    log->flush_on(config->GetFlushLevel());

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%-8l] [%t] [%s:%!:%#] %v");
}

SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    auto *pConfig = LIBC_NAMESPACE::SimpleIME::ImeApp::LoadConfig();
    InitializeLogging(pConfig);

    auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
    auto  version = plugin->GetVersion();
    logv(info, "{} {} is loading...", plugin->GetName(), version.string());

    SKSE::Init(skse);
    LIBC_NAMESPACE::SimpleIME::ImeApp::Init();

    logv(info, "{} has finished loading.", plugin->GetName());
    return true;
}