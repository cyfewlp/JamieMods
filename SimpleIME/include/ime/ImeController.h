#pragma once

#include "IImeModule.h"
#include "ime/ImeManager.h"
#include "ui/Settings.h"
#include "ui/TaskQueue.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

class ImeWnd;

struct SettingsConfig;

class ImeController final
{
public:
    void ApplyUiSettings(const Settings &settings);

    void SyncImeStateIfDirty()
    {
        if (m_fDirty)
        {
            SyncImeState();
        }
    }

    void MarkDirty()
    {
        m_fDirty = true;
    }

    [[nodiscard]] constexpr auto IsDirty() const -> bool
    {
        return m_fDirty;
    }

    auto IsModEnabled() const -> bool
    {
        return m_settings->enableMod;
    }

    //////////////////////////////////////////////////

    auto EnableIme(bool enable) const -> void;
    auto ForceFocusIme() const -> void;
    auto SyncImeState() -> void;
    auto TryFocusIme() const -> void;
    auto EnableMod(bool enable) -> void;

    [[nodiscard]] auto IsInited() const -> bool
    {
        return m_fInited;
    }

    static auto GetInstance() -> ImeController *
    {
        static ImeController g_instance;
        return &g_instance;
    }

private:
    Settings                   *m_settings = nullptr;
    std::unique_ptr<ImeManager> m_delegate = nullptr;
    ImeWnd                     *m_imeWnd   = nullptr;
    HWND                        m_gameHwnd = nullptr;
    bool                        m_fDirty   = false;
    std::atomic_bool            m_fInited  = false;

    auto DoEnableMod(bool enable) const -> IImeModule::Result;
    auto DoEnableIme(bool enable) const -> IImeModule::Result;
    auto DoForceFocusIme() const -> IImeModule::Result;
    auto DoSyncImeState() -> IImeModule::Result;
    auto DoTryFocusIme() const -> IImeModule::Result;

    auto UnlockKeyboard() const -> bool;
    auto RestoreKeyboard() const -> bool;
    bool FocusImeOrGame(bool focusIme) const;

    void AddTask(TaskQueue::Task &&task) const;

    friend class ImeWnd;

    static void Init(ImeWnd *imeWnd, HWND gameHwnd, Settings &settings);
};
}
}