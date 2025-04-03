#pragma once

#include "PapyrusEvent.h"

#include <RE/R/Renderer.h>
#include <stdexcept>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    class SosGui
    {
        struct InitFail : std::runtime_error
        {
            explicit InitFail(const char *msg) : std::runtime_error(msg)
            {
            }
        };

        bool m_fShow = false;

    public:
        static auto GetInstance() -> SosGui &
        {
            static SosGui g_instance;
            return g_instance;
        }

        static auto Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool;

        auto Render() -> void;
        auto ToggleShow() -> void;

    private:
        static void NewFrame();

        auto DoRender() -> void;
        void RenderCharactersConfig();
        void RenderCharactersList();
        void RenderLocationBasedAutoswitch();
    };
}