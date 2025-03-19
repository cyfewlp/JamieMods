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
        // enable IME for other mod
        bool (*EnableIme)(bool enable) = nullptr;
        // update IME window position (the candidate and composition window)
        void (*UpdateImeWindowPosition)(float posX, float posY) = nullptr;
        // Is IME enabled?(in candidate choose, composition)
        bool (*IsEnabled)() = nullptr;
    };

    static_assert(sizeof(IntegrationData) == 32);
}