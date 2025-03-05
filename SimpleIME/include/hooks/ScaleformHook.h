//
// Created by jamie on 2025/3/2.
//

#ifndef HOOK_H
#define HOOK_H

#include "common/hook.h"
#include "common/log.h"

#include "hooks/Hooks.hpp"

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        struct Scaleform_SetScaleModeTypeHookData : HookData<void(RE::GFxMovieView *, RE::GFxMovieView::ScaleModeType)>
        {
            // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
            explicit Scaleform_SetScaleModeTypeHookData(func_type *ptr)
                : HookData(REL::RelocationID(80302, 82325),        //
                           REL::VariantOffset(0x1D9, 0x1DD, 0x00), //
                           ptr, true)
            {
                log_debug("GfxMovieInstall hooked at {:#x}", m_address);
            }

            // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
        };

        class ScaleformAllowTextInput final : public RE::GFxFunctionHandler
        {
            static inline std::uint8_t g_textEntryCount = 0;

        public:
            void Call(Params &params) override;

            static auto AllowTextInput(bool allow) -> std::uint8_t;

            static constexpr auto TextEntryCount() -> std::uint8_t
            {
                return g_textEntryCount;
            }
        };

        class ScaleformHooks
        {
            static inline std::unique_ptr<Scaleform_SetScaleModeTypeHookData> g_SetScaleModeTypeHook = nullptr;
            static void SetScaleModeTypeHook(RE::GFxMovieView *pMovieView, RE::GFxMovieView::ScaleModeType scaleMode);

        public:
            static void InstallHooks();
        };
    }
}

#endif // HOOK_H
