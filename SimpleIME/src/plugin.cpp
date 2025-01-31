//
// Created by jamie on 25-1-22.
//
#include "ImeApp.h"

static void InitializeLogging(bool debug)
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
    if (debug)
    {
        log->set_level(spdlog::level::debug);
    }
    log->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%-8l] [%t] [%s:%!:%#] %v");
}

SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    auto *pConfig = SimpleIME::ImeApp::LoadConfig();
    InitializeLogging(pConfig->debug);

    auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
    auto  version = plugin->GetVersion();
    logv(info, "{} {} is loading...", plugin->GetName(), version.string());

    SKSE::Init(skse);
    SimpleIME::ImeApp::Init();

    logv(info, "{} has finished loading.", plugin->GetName());
    return true;
}