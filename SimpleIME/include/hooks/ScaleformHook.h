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

        static constexpr uint8_t MAX_TEXT_ENTRY_COUNT = 0xFF;

        class ControlMap : public RE::BSTSingletonSDM<ControlMap>,         // 00
                           public RE::BSTEventSource<RE::UserEventEnabled> // 08
        {
        public:
            RE::ControlMap::InputContext                *controlMap[RE::ControlMap::InputContextID::kTotal]; // 060
            RE::BSTArray<RE::ControlMap::LinkedMapping>  linkedMappings;                                     // 0E8
            RE::BSTArray<RE::ControlMap::InputContextID> contextPriorityStack;                               // 100
            uint32_t                                     enabledControls;                                    // 118
            uint32_t                                     unk11C;                                             // 11C
            std::int8_t                                  textEntryCount;                                     // 120
            bool                                         ignoreKeyboardMouse;                                // 121
            bool                                         ignoreActivateDisabledEvents;                       // 122
            std::uint8_t                                 pad123;                                             // 123
            uint32_t                                     gamePadMapType;                                     // 124
            uint8_t                                      allowTextInput;                                     // 128
            uint8_t                                      unk129;                                             // 129
            uint8_t                                      unk12A;                                             // 12A
            uint8_t                                      pad12B;                                             // 12B
            uint32_t                                     unk12C;                                             // 12C

            // A SKSE copy: InputManager::AllowTextInput
            auto SKSE_AllowTextInput(bool allow) -> uint8_t;

            static ControlMap *GetSingleton(void);
        };

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

            // call SKSE_AllowTextInput to allow and return its result
            static auto AllowTextInput(bool allow) -> std::uint8_t;
            // use our text-entry-count
            static void OnTextEntryCountChanged();

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

            static void UninstallHooks();
        };
    }
}

#endif // HOOK_H
