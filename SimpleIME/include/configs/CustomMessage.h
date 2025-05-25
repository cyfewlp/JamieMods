#ifndef CONFIGS_CONFIGS_H
#define CONFIGS_CONFIGS_H

#pragma once

enum
{
    WM_CUSTOM = 0x7000
};

namespace LIBC_NAMESPACE_DECL
{
enum CustomMessage
{
    CM_CHAR = WM_CUSTOM + 1,
    CM_IME_CHAR,
    CM_IME_COMPOSITION,
    CM_EXECUTE_TASK,
    CM_ACTIVATE_PROFILE // notify active profile, guid passed in LPARAM
};
} // namespace LIBC_NAMESPACE_DECL

#endif