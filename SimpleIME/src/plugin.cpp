//
// Created by jamie on 25-1-22.
//
#include "common/common.h"
#include "common/log.h"
#include "configs/AppConfig.h"

#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <stacktrace>

namespace LIBC_NAMESPACE_DECL
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
            const auto *plugin         = SKSE::PluginDeclaration::GetSingleton();
            auto        configFilePath = std::format(R"(Data\SKSE\Plugins\{}.ini)", plugin->GetName());
            Ime::AppConfig::LoadIni(configFilePath.c_str());

            const auto &pConfig = Ime::AppConfig::GetConfig();
            InitializeLogging(pConfig.GetLogLevel(), pConfig.GetFlushLevel());

            Init(skse);

            const auto version = plugin->GetVersion();
            log_info("{} {} is loading...", plugin->GetName(), version.string());

            PluginInit();

            log_info("{} has finished loading.", plugin->GetName());
            return true;
        }
        catch (std::exception &exception)
        {
            log_error("Fatal error, SimpleIME init fail: {}", exception.what());
            LogStacktrace();
        }
        catch (...)
        {
            log_error("Fatal error. occur unknown exception.");
            LogStacktrace();
        }
        return false;
    }

    int ErrorHandler(unsigned int code, _EXCEPTION_POINTERS *)
    {
        log_critical("System exception (code {}) raised during plugin initialization.", code);
        LogStacktrace();
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse)
{
    __try
    {
        return LIBC_NAMESPACE::PluginLoad(skse);
    }
    __except (LIBC_NAMESPACE::ErrorHandler(GetExceptionCode(), GetExceptionInformation()))
    {
    }
    return false;
}