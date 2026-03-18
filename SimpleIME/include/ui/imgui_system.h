//
// Created by jamie on 2026/1/16.
//
#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <windef.h>

struct ImFont;

namespace ImGuiEx::M3
{
struct SchemeConfig;
}

namespace Ime::UI
{
void Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context);

/**
 * Add the primary font, supporting the passing of multiple fonts as fallback fonts, which will be automatically
 * merged and set as the default font
 * @param fontsPathList the primary font and fallback fonts file path list
 * @param fallbackFontsPathList if the primary font load failed, will fallback to these fonts in order. If the primary font load successfully, these
 * fonts will be ignored. Can be empty.
 * @return the ImFont pointer.
 */
auto AddPrimaryFont(const std::vector<std::string> &fontsPathList, const std::vector<std::string> &fallbackFontsPathList) -> ImFont *;

/**
 * Add an independent ICON font.
 */
[[nodiscard]] auto AddFont(const std::string &filePath) -> ImFont *;

/**
 * @brief Initialize Material3 style.
 * Should be called after initializing ImGui context and before the first frame.
 * @param iconFontPath [REQUIRED] if the icon font load failed, will fallback to default font and cause icons not showing, so it's required to provide
 * a valid icon font path.
 * @param schemeConfig [REQUIRED] be used to initialize M3Styles ColorScheme.
 */
void InitializeM3(const std::filesystem::path &iconFontPath, const ImGuiEx::M3::SchemeConfig &schemeConfig);
void DestroyM3();

void NewFrame();

void Render();

void EndFrame();

void Shutdown();

} // namespace Ime::UI
