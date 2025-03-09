//
// Created by jamie on 2025/3/3.
//

#ifndef UIADDMESSAGEHOOK_H
#define UIADDMESSAGEHOOK_H

#include "common/hook.h"

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
                log_debug("UiAddMessageHookData hooked at {:#x}", m_address);
            }
        };

        inline const RE::BSFixedString IME_MESSAGE_FAKE_MENU = "ImeMessageFakeMenu";

        class UiHooks
        {
            static inline std::unique_ptr<UiAddMessageHookData> UiAddMessage           = nullptr;
            static inline bool                                  g_fEnableMessageFilter = false;

        public:
            static void InstallHooks();

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
        };

    }
}

#endif // UIADDMESSAGEHOOK_H
