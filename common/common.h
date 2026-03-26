#pragma once

namespace SksePlugin
{
#ifdef SPDLOG_API
    #include <spdlog/common.h>

struct SpdLogSettings
{
    spdlog::level::level_enum level;
    spdlog::level::level_enum flushLevel;
};

void InitializeLogging(SpdLogSettings settings);
#endif

auto Initialize() -> bool;

auto PluginLoad(const SKSE::LoadInterface *skse) -> bool;

void InitializeMessaging();

auto ErrorHandler(unsigned int code, _EXCEPTION_POINTERS *) -> int;
} // namespace SksePlugin
