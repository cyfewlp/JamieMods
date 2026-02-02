#pragma once


namespace SksePlugin
{
#ifdef SPDLOG_API
#include <spdlog/common.h>

void InitializeLogging(spdlog::level::level_enum logLevel, spdlog::level::level_enum flushLevel);
#endif


auto Initialize() -> bool;

void InitializeMessaging();

auto ErrorHandler(unsigned int code, _EXCEPTION_POINTERS *) -> int;
}