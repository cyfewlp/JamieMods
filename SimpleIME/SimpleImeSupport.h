#pragma once
#include <cstdint>

#ifdef SIMPLE_EXPORTS
    #define SIMPLE_API __declspec(dllexport)
#else
    #define SIMPLE_API __declspec(dllimport)
#endif

namespace SimpleIME
{
    enum class SkseImeMessage
    {
        IME_INTEGRATION_INIT = 0x100,
        IME_COMPOSITION_RESULT,
    };

    struct SIMPLE_API IntegrationData
    {
        // Other mod must call this to render IME window
        /// Note: If current not exists context, this method will be ignore.
        void (*RenderIme)() = nullptr;

        /// <summary>
        /// Try enable IME.
        /// Must use IsWantCaptureInput to check current state because
        /// IME enabled state is updated asynchronously.
        ///
        /// Note: If SimpleIME "Enable mod" unchecked or current not exists context,
        /// this method will be fail(return false)
        /// </summary>
        bool (*EnableIme)(bool enable) = nullptr;

        /// <summary>
        /// Notify SimpleIME a mod menu is open.
        /// SimpleIME needs to use this to disable IME for the game menu
        /// Currently, this method simply increases a reference count that other mods using SimpleIME increment.
        ///
        /// Note: PushContext and PopContext must be called in pairs. Whether the "EnableMod" option is checked in
        /// SimpleIME does not affect their execution. These methods should be invoked when a third - party UI mod's
        /// menu opens and closes respectively.
        /// </summary>
        /// <returns>the reference count previously</returns>
        uint32_t (*PushContext)() = nullptr;

        /// <summary>
        /// Notify SimpleIME a mod menu is closed
        /// SimpleIME need to use this to enable IME for game menu.
        /// Currently, this method simply decrease a reference count that other mods using SimpleIME increment.
        ///
        /// Note: PushContext and PopContext must be called in pairs. Whether the "EnableMod" option is checked in
        /// SimpleIME does not affect their execution. These methods should be invoked when a third - party UI mod's
        /// menu opens and closes respectively.
        /// </summary>
        /// <returns>the reference count previously</returns>
        uint32_t (*PopContext)() = nullptr;

        // update IME window position (the candidate and composition window)
        // Note: If SimpleIME "Enable mod" unchecked or current not exists context, this method will be ignore.
        void (*UpdateImeWindowPosition)(float posX, float posY) = nullptr;

        /// <summary>
        //  Check current IME want to capture user keyboard input?
        //  Note: iFly won't update conversion mode value
        /// </summary>
		/// <param name="keycode">must be DIK code(scan code)</param>
        /// <returns>return true if SimpleIME mod enabled and IME not in alphanumeric mode,
        /// otherwise, return false.
        /// </returns>
        bool (*IsWantCaptureInput)(uint32_t keyCode) = nullptr;
    };

    static_assert(sizeof(IntegrationData) == 48);
}