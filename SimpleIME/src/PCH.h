#pragma once

// This file is required.

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#define LIBC_NAMESPACE __llvm_libc_simpleime

#include "common/config.h" // NOLINT

using namespace std::literals;

static constexpr int DIRECTINPUT_ACQUIRE_FAILED = 0x1;

class SimpleIMEException : public std::runtime_error
{
public:
    int code;

    explicit SimpleIMEException(const std::string &_Message, int a_code = 0) : code(a_code), runtime_error(_Message) {}
};
