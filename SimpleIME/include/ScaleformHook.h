//
// Created by jamie on 2025/3/2.
//

#ifndef HOOK_H
#define HOOK_H

#include "common/hook.h"
#include "common/log.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        struct GfxMovieInstallHookData : HookData<void(RE::GFxMovieView *, RE::GFxMovieView::ScaleModeType)>
        {
            explicit GfxMovieInstallHookData(func_type *ptr)
                : HookData(REL::RelocationID(80302, 82325),        //
                           REL::VariantOffset(0x1D9, 0x1DD, 0x00), //
                           ptr, true)
            {
                log_debug("GfxMovieInstall hooked at {:#x}", m_address);
            }
        };

        class ScaleformAllowTextInput : public RE::GFxFunctionHandler
        {
            static inline std::uint8_t g_textEntryCount = 0;

        public:
            void Call(Params &params) override;

            static void AllowTextInput(bool allow);
        };

        class ScaleformHooks
        {
            static inline std::unique_ptr<GfxMovieInstallHookData> LoadMovieHook = nullptr;
            static void GfxMovieInstallHook(RE::GFxMovieView *pMovieView, RE::GFxMovieView::ScaleModeType scaleMode);

        public:
            static void InstallHooks();
        };

        inline std::unique_ptr<GfxMovieInstallHookData> LoadMovieHook = nullptr;

    }
}

#endif // HOOK_H
