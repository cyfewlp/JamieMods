#pragma once

namespace Ime
{
enum CustomMessage
{
    WM_CUSTOM = 0x7000,
    CM_CHAR = WM_CUSTOM + 1,
    CM_IME_CHAR,
    CM_IME_COMPOSITION,
    CM_EXECUTE_TASK,
    CM_ACTIVATE_PROFILE // notify active profile, guid passed in LPARAM
};
}