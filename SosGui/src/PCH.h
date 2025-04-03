#pragma once

// This file is required.

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#define LIBC_NAMESPACE __llvm_libc_sosgui

#include "common/config.h" // NOLINT

using namespace std::literals;