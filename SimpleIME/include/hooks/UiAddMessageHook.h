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

        struct UiAddMessageHookData : FunctionHook<void(RE::UIMessageQueue*, RE::BSFixedString &, RE::UI_MESSAGE_TYPE, RE::IUIMessageData *)>
        {
            explicit UiAddMessageHookData(func_type *ptr) : FunctionHook(RE::Offset::UIMessageQueue::AddMessage, ptr)
            {
                log_debug("UiAddMessageHookData hooked at {:#x}", m_address);
            }
        };

        inline std::unique_ptr<UiAddMessageHookData> UiAddMessageHook = nullptr;
    }
}

#endif // UIADDMESSAGEHOOK_H
