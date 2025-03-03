//
// Created by jamie on 2025/3/3.
//

#ifndef UIADDMESSAGEHOOK_H
#define UIADDMESSAGEHOOK_H

#include "common/hook.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {

        template <typename func_t>
            requires requires {
                std::is_function_v<func_t>;
                std::is_pointer_v<func_t>;
            }
        class FunctionHook
        {
        };

        class DetourUtil
        {
        public:
            static auto DetourAttach(void **original, void *hook) -> bool;
            static auto DetourDetach(void **original, void *hook) -> bool;
        };

        template <typename Return, typename... Args>
        class FunctionHook<Return(Args...)>
        {
            void *m_originalFuncPtr;
            void *m_hook;
            bool  detoured = false;

        protected:
            std::uintptr_t m_address;
            using func_type = Return(Args...);

        public:
            explicit FunctionHook(REL::RelocationID id, Return (*funcPtr)(Args...))
            {
                m_address         = id.address();
                m_originalFuncPtr = reinterpret_cast<void *>(m_address);
                m_hook            = reinterpret_cast<void *>(funcPtr);

                if (DetourUtil::DetourAttach(&m_originalFuncPtr, m_hook))
                {
                    detoured = true;
                }
            }

            ~FunctionHook()
            {
                if (detoured)
                {
                    DetourUtil::DetourDetach(&m_originalFuncPtr, m_hook);
                }
            }

            [[nodiscard]] bool Detoured() const
            {
                return detoured;
            }

            constexpr Return operator()(Args... args) const
            {
                if constexpr (std::is_void_v<Return>)
                {
                    reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
                else
                {
                    return reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
            }

            constexpr Return Original(Args... args) const
            {
                if constexpr (std::is_void_v<Return>)
                {
                    reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
                else
                {
                    return reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
            }
        };

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
