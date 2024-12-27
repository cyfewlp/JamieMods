#pragma once

// This file is required.

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#define LOG(a_type, ...) \
    spdlog::log(spdlog::source_loc(__FILE__, __LINE__, __FUNCTION__), spdlog::level::a_type, __VA_ARGS__)

using namespace std::literals;
