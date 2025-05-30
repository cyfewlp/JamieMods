#pragma once

#include "ime/BaseImeManager.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
// All operations call in the main thread
class TemporaryFocusImeManager : public BaseImeManager
{
    using State = Core::State;

public:
    TemporaryFocusImeManager(ImeWnd *pImeWnd, HWND gameHwnd, Settings &settings)
        : BaseImeManager(gameHwnd, settings), m_ImeWnd(pImeWnd)
    {
    }

    ~TemporaryFocusImeManager() override = default;

protected:
    auto DoEnableIme(bool enable) -> bool override;
    auto DoEnableMod(bool enable) -> bool override;
    auto DoForceFocusIme() -> bool override;
    auto DoSyncImeState() -> bool override;
    auto DoTryFocusIme() -> bool override;

private:
    ImeWnd *m_ImeWnd;
    bool    m_fIsInEnableIme = false;
};
}
}