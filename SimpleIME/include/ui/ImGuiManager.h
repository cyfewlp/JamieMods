//
// Created by jamie on 2026/1/16.
//
#pragma once

#include "Settings.h"
#include "common/config.h"

#include <d3d11.h>
#include <windef.h>

struct ImFont;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImGuiManager
{
    inline static bool g_initialized = false;

public:
    static void Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context, Settings &settings);

    static void NewFrame();

    static void Render();

    static void EndFrame(Settings &settings);

    static void Shutdown();

private:
    static void InitFonts(const Settings &settings);
    static auto AddFonts(const std::vector<std::string> &fontPaths) -> ImFont *;
    static void UpdateCursorPos();
    static void EnableTextInputIfNeed();
};
}
}
