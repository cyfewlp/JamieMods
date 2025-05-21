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

    static inline std::atomic_bool g_fEnableMod = false;

public:
    ImeManager()                        = default;
    virtual ~ImeManager()               = default;
    ImeManager(const ImeManager &other) = delete;
    ImeManager(ImeManager &&other)      = delete;

    virtual auto EnableIme(bool enable) -> bool             = 0;
    virtual auto EnableMod(bool enable) -> bool             = 0;
    virtual auto GiveUpFocus() const -> bool                = 0;
    virtual auto ForceFocusIme() -> bool                    = 0;
    virtual auto SyncImeState() -> bool                     = 0;
    /// <summary>
    /// Try focus IME when mod is enabled
    /// </summary>
    /// <returns>true if mod enabled and focus success, otherwise false</returns>
    virtual auto TryFocusIme() -> bool = 0;

    static auto Focus(HWND hwnd) -> bool;
    static auto UnlockKeyboard() -> bool;
    static auto RestoreKeyboard() -> bool;

    [[nodiscard]] static constexpr auto IsModEnabled() -> bool
    {
        return g_fEnableMod;
    }

    static void SetEnableMod(const bool enableMod)
    {
        g_fEnableMod = enableMod;
    }
};
}
}