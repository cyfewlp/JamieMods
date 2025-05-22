//
// Created by jamie on 2025/3/2.
//

#ifndef HOOK_H
#define HOOK_H

#include "common/hook.h"
#include "common/log.h"
#include "hooks/Hooks.hpp"

#include <memory>

namespace LIBC_NAMESPACE_DECL
{
namespace Hooks
{

static constexpr uint8_t MAX_TEXT_ENTRY_COUNT = 0xFF;

class ControlMap : public RE::BSTSingletonSDM<ControlMap>,         // 00
                   public RE::BSTEventSource<RE::UserEventEnabled> // 08
{
    using CM = RE::ControlMap;

public:
    // NOLINTBEGIN( misc-non-private-member-variables-in-classes)
    CM::InputContext                *controlMap[CM::InputContextID::kTotal]; // 060
    RE::BSTArray<CM::LinkedMapping>  linkedMappings;                         // 0E8
    RE::BSTArray<CM::InputContextID> contextPriorityStack;                   // 100
    uint32_t                         enabledControls;                        // 118
    uint32_t                         unk11C;                                 // 11C
    std::uint8_t                     textEntryCount;                         // 120
    bool                             ignoreKeyboardMouse;                    // 121
    bool                             ignoreActivateDisabledEvents;           // 122
    std::uint8_t                     pad123;                                 // 123
    uint32_t                         gamePadMapType;                         // 124
    uint8_t                          allowTextInput;                         // 128
    uint8_t                          unk129;                                 // 129
    uint8_t                          unk12A;                                 // 12A
    uint8_t                          pad12B;                                 // 12B
    uint32_t                         unk12C;                                 // 12C
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    // A SKSE copy: InputManager::AllowTextInput
    auto SKSE_AllowTextInput(bool allow) -> uint8_t;

    static ControlMap *GetSingleton(void);

private:
    static void DoAllowTextInput(bool allow, std::uint8_t &entryCount);
};

class Scaleform_SetScaleModeTypeHookData : public HookData<void(RE::GFxMovieView *, RE::GFxMovieView::ScaleModeType)>
{
public:
    // NOLINTBEGIN(*-magic-numbers)
    explicit Scaleform_SetScaleModeTypeHookData(func_type *ptr)
        : HookData(
              REL::RelocationID(80302, 82325),        //
              REL::VariantOffset(0x1D9, 0x1DD, 0x00), //
              ptr, true
          )
    {
        log_debug("Installed {}: ", __func__, ToString());
    }

    // NOLINTEND(*-magic-numbers)
};

class Scaleform_LoadMovieHook : public FunctionHook<bool(
                                    RE::BSScaleformManager *, RE::IMenu *, RE::GPtr<RE::GFxMovieView> &, const char *,
                                    RE::BSScaleformManager::ScaleModeType, float
                                )>
{
public:
    // NOLINTBEGIN(*-magic-numbers)
    explicit Scaleform_LoadMovieHook(func_type *ptr) : FunctionHook(REL::RelocationID(80302, 82325), ptr)
    {
        log_debug("Installed {}: ", __func__, ToString());
    }

    // NOLINTEND(*-magic-numbers)
};

void UpdateFocusCharacterBound(RE::GFxMovieView *movieView, bool allow);

class Scaleform_AllowTextInput : public FunctionHook<uint8_t(ControlMap *, bool)>
{
public:
    // NOLINTBEGIN(*-magic-numbers)
    explicit Scaleform_AllowTextInput(func_type *ptr) : FunctionHook(REL::RelocationID(67252, 68552), ptr)
    {
        log_debug("Installed {}: ", __func__, ToString());
    }

    // NOLINTEND(*-magic-numbers)
};

class SKSE_ScaleformAllowTextInput final : public RE::GFxFunctionHandler
{
    static inline std::uint8_t g_textEntryCount = 0;

public:
    void Call(Params &params) override;

    // call SKSE_AllowTextInput to allow and return its result
    static auto AllowTextInput(bool allow) -> std::uint8_t;
    // use our text-entry-count
    static void OnTextEntryCountChanged(std::uint8_t entryCount);

    static constexpr auto HasTextEntry() -> bool
    {
        return g_textEntryCount > 0;
    }

    static constexpr auto TextEntryCount() -> std::uint8_t
    {
        return g_textEntryCount;
    }
};

class ScaleformHooks
{
    std::unique_ptr<Scaleform_SetScaleModeTypeHookData> m_SetScaleModeTypeHook = nullptr;
    std::unique_ptr<Scaleform_LoadMovieHook>            m_LoadMovieHook        = nullptr;
    std::unique_ptr<Scaleform_AllowTextInput>           m_AllowTextInputHook   = nullptr;

    static void SetScaleModeTypeHook(RE::GFxMovieView *pMovieView, RE::GFxMovieView::ScaleModeType scaleMode);
#ifdef HOOK_LOAD_MOVIE
    static bool LoadMovieHook(
        RE::BSScaleformManager *, RE::IMenu *, RE::GPtr<RE::GFxMovieView> &, const char *,
        RE::BSScaleformManager::ScaleModeType, float
    );
#endif
    // Game default AllowTextInout
    static auto Scaleform_AllowTextInputHook(ControlMap *self, bool allow) -> uint8_t;

    ScaleformHooks();

public:
    static void Install();

    static void Uninstall();
};
}
}

#endif // HOOK_H
