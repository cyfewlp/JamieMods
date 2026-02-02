#pragma once

// This file is required.

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#define LIBC_NAMESPACE __llvm_libc_simpleime

// NOLINT

using namespace std::literals;

static constexpr auto SIMPLE_IME = "SimpleIME";

class SimpleIMEException : public std::runtime_error
{
public:
    int code;

    explicit SimpleIMEException(const std::string &_Message, int a_code = 0) : runtime_error(_Message), code(a_code) {}
};
