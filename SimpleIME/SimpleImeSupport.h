#pragma once

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
        void (*RenderIme)() = nullptr;

        /// <summary>
        /// Try enable IME.
        /// Must use IsWantCaptureInput to check current state because
        /// IME enabled state is updated asynchronously.
        /// </summary>
        bool (*EnableIme)(bool enable) = nullptr;

        // update IME window position (the candidate and composition window)
        void (*UpdateImeWindowPosition)(float posX, float posY) = nullptr;

        /// <summary>
        //  Check current IME want to capture user keyboard input?
        //  Note: iFly won't update conversion mode value
        /// </summary>
        /// <returns>return true if SimpleIME mod enabled and IME not in alphanumeric mode,
        /// otherwise, return false.
        /// </returns>
        bool (*IsWantCaptureInput)() = nullptr;
    };

    static_assert(sizeof(IntegrationData) == 32);
}