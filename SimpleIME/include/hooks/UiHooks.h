//
// Created by jamie on 2025/3/3.
//

#ifndef UIADDMESSAGEHOOK_H
#define UIADDMESSAGEHOOK_H

#include "RE/GFxCharEvent.h"
#include "common/log.h"
#include "hooks/Hooks.hpp"
#include "ui/Settings.h"

#include <atomic>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
namespace Hooks
{

struct UiAddMessageHook
    : FunctionHook<void(RE::UIMessageQueue *, RE::BSFixedString &, RE::UI_MESSAGE_TYPE, RE::IUIMessageData *)>
{
    explicit UiAddMessageHook(func_type *ptr) : FunctionHook(RE::Offset::UIMessageQueue::AddMessage, ptr)
    {
        log_debug("Installed {}: ", __func__, ToString());
    }
};

struct MenuProcessMessageHook : FunctionHook<RE::UI_MESSAGE_RESULTS(RE::IMenu *, RE::UIMessage &)>
{
    explicit MenuProcessMessageHook(func_type *ptr) : FunctionHook(RELOCATION_ID(80283, 82306), ptr)
    {
        log_debug("Installed {}: ", __func__, ToString());
    }
};

struct ConsoleProcessMessageHook : FunctionHook<RE::UI_MESSAGE_RESULTS(RE::IMenu *, RE::UIMessage &)>
{
    explicit ConsoleProcessMessageHook(func_type *ptr) : FunctionHook(RELOCATION_ID(50155, 442669), ptr)
    {
        log_debug("Installed {}: ", __func__, ToString());
    }
};

class UiHooks
{
    std::unique_ptr<UiAddMessageHook>          UiAddMessage           = nullptr;
    std::unique_ptr<MenuProcessMessageHook>    MenuProcessMessage     = nullptr;
    std::unique_ptr<ConsoleProcessMessageHook> ConsoleProcessMessage  = nullptr;
    std::atomic_bool                           m_fEnableMessageFilter = false;
    Ime::Settings                             &m_settings;

    explicit UiHooks(Ime::Settings &settings) : m_settings(settings) {}

public:
    static void Install(Ime::Settings &settings);

    static void Uninstall();

    static auto GetInstance() -> UiHooks *;

    void EnableMessageFilter(bool enable)
    {
        m_fEnableMessageFilter = enable;
    }

    auto IsEnableMessageFilter() -> bool
    {
        return m_fEnableMessageFilter;
    }

private:
    static void AddMessageHook(
        RE::UIMessageQueue *self, RE::BSFixedString &menuName, RE::UI_MESSAGE_TYPE messageType,
        RE::IUIMessageData *pMessageData
    );

    // use GFxMovieView::handleEvent paset text;
    // reuse the input CharEvent;
    static void ScaleformPasteText(RE::GFxMovieView *const uiMovie, RE::GFxCharEvent *const charEvent);

    // Handle Ctrl-V: if mod enabled paste, disable game do paste operation.
    // And do our paste operation after the original function return.
    static auto MyMenuProcessMessage(RE::IMenu *, RE::UIMessage &) -> RE::UI_MESSAGE_RESULTS;

    static auto MyConsoleProcessMessage(RE::IMenu *, RE::UIMessage &) -> RE::UI_MESSAGE_RESULTS;
};

}
}

#endif // UIADDMESSAGEHOOK_H
