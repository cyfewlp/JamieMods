#pragma once

#include "common/config.h"
#include "core/State.h"

#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImeWnd;

class ImeManager
{
    using State = Core::State;
    HWND m_gameHwnd;

public:
    ImeManager(HWND hwnd) : m_gameHwnd(hwnd) {}

    virtual ~ImeManager()               = default;
    ImeManager(const ImeManager &other) = delete;
    ImeManager(ImeManager &&other)      = delete;

    virtual auto EnableIme(bool enable) -> bool = 0;
    virtual auto EnableMod(bool enable) -> bool = 0;
    virtual auto GiveUpFocus() const -> bool    = 0;
    virtual auto ForceFocusIme() -> bool        = 0;
    virtual auto SyncImeState() -> bool         = 0;
    /// <summary>
    /// Try to focus IME when mod is enabled
    /// </summary>
    /// <returns>true if mod enabled and focus success, otherwise false</returns>
    virtual auto TryFocusIme() -> bool = 0;

    static auto Focus(HWND hwnd) -> bool;
    auto        UnlockKeyboard() const -> bool;
    auto        RestoreKeyboard() const -> bool;

    auto GetGameHwnd() const -> HWND
    {
        return m_gameHwnd;
    }
};
}
}