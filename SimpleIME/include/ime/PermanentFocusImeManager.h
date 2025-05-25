#pragma once

#include "ime/BaseImeManager.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

class PermanentFocusImeManager final : public BaseImeManager
{
    using State = Core::State;

public:
    PermanentFocusImeManager(ImeWnd *pImeWnd, HWND gameHwnd, Settings &settings)
        : BaseImeManager(gameHwnd, settings), m_ImeWnd(pImeWnd)
    {
    }

    ~PermanentFocusImeManager() override = default;

protected:
    auto DoEnableIme(bool enable) -> bool override;
    auto DoEnableMod(bool enable) -> bool override;
    auto DoForceFocusIme() -> bool override;
    auto DoSyncImeState() -> bool override;
    auto DoTryFocusIme() -> bool override;

private:
    ImeWnd *m_ImeWnd;

    auto FocusImeOrGame(bool focusIme) const -> bool;
};
}
}