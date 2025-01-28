#pragma once

// This file is required.

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <windows.h>

using namespace std::literals;

#define logv(a_type, ...)                                                                                              \
    Logger::log(spdlog::source_loc(__FILE__, __LINE__, __FUNCTION__), spdlog::level::a_type, __VA_ARGS__)

struct Logger
{
    template <typename T> static void log(spdlog::source_loc loc, spdlog::level::level_enum level, const T &value)
    {
        spdlog::log(loc, level, value);
    }
    template <typename... Types>
    static void log(spdlog::source_loc loc, spdlog::level::level_enum level, const char *const message,
                    const Types &...params)
    {
        auto fmt = std::vformat(std::string(message), std::make_format_args(params...));
        spdlog::log(loc, level, fmt.c_str());
    }
};