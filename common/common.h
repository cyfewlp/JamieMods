#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#pragma once

#ifdef LIBC_NAMESPACE_DECL
namespace LIBC_NAMESPACE_DECL
{
    void InitializeLogging(spdlog::level::level_enum logLevel, spdlog::level::level_enum flushLevel);

    auto PluginInit() -> bool;

    void InitializeMessaging();

    auto ErrorHandler(unsigned int code, _EXCEPTION_POINTERS *) -> int;
} // namespace LIBC_NAMESPACE_DECL
#endif // LIBC_NAMESPACE_DECL

#endif
