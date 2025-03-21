//
// Created by jamie on 2025/3/3.
//

#ifndef UIADDMESSAGEHOOK_H
#define UIADDMESSAGEHOOK_H

#include "common/log.h"

#include "core/State.h"
#include "hooks/Hooks.hpp"

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {

        struct UiAddMessageHookData
            : FunctionHook<void(RE::UIMessageQueue *, RE::BSFixedString &, RE::UI_MESSAGE_TYPE, RE::IUIMessageData *)>
        {
            explicit UiAddMessageHookData(func_type *ptr) : FunctionHook(RE::Offset::UIMessageQueue::AddMessage, ptr)
            {
                log_debug("{} hooked at {:#x}", __func__, m_address);
            }
        };

        struct MenuProcessMessageHook : public FunctionHook<RE::UI_MESSAGE_RESULTS(RE::IMenu *, RE::UIMessage &)>
        {
            // Console#ProcessMessage FunctionHook(RELOCATION_ID(442669, 442669)
            explicit MenuProcessMessageHook(func_type *ptr) : FunctionHook(RELOCATION_ID(80283, 82306), ptr)
            {
                log_debug("{} hooked at {:#x}", __func__, m_address);
            }
        };

        inline const RE::BSFixedString IME_MESSAGE_FAKE_MENU = "ImeMessageFakeMenu";

        class UiHooks
        {
            static inline std::unique_ptr<UiAddMessageHookData>   UiAddMessage           = nullptr;
            static inline std::unique_ptr<MenuProcessMessageHook> ConsoleProcessMessage  = nullptr;
            static inline std::atomic_bool                        g_fEnableMessageFilter = false;

        public:
            static void InstallHooks();

            static void UninstallHooks();

            static void EnableMessageFilter(bool enable)
            {
                g_fEnableMessageFilter = enable;
            }

            static auto IsEnableMessageFilter() -> bool
            {
                return g_fEnableMessageFilter;
            }

        private:
            static void AddMessageHook(RE::UIMessageQueue *self, RE::BSFixedString &menuName,
                                       RE::UI_MESSAGE_TYPE messageType, RE::IUIMessageData *pMessageData);

            static auto IsEnableUnicodePaste() -> bool
            {
                auto &state = Ime::Core::State::GetInstance();
                return state.IsModEnabled() && state.IsEnableUnicodePaste();
            }

            // Handle Ctrl-V: if mod enabled paste, disable game do paste operation.
            // And do our paste operation after the original function return.
            static auto MyConsoleProcessMessage(RE::IMenu *, RE::UIMessage &) -> RE::UI_MESSAGE_RESULTS;
        };

    }
}

#endif // UIADDMESSAGEHOOK_H
