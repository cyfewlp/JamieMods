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
class M3Styles;
}

namespace Ime::UI
{
void Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context);

/**
 * Add the primary font, supporting the passing of multiple fonts as fallback fonts, which will be automatically
 * merged and set as the default font
 * @param fontsPathList the primary font and fallback fonts file path list
 * @return the ImFont pointer.
 */
auto AddPrimaryFont(const std::vector<std::string> &fontsPathList) -> ImFont *;

/**
 * Add an independent ICON font.
 */
[[nodiscard]] auto AddFont(const std::string &filePath) -> ImFont *;

void NewFrame();

void Render();

void EndFrame();

void Shutdown();

void ApplyM3Theme(const ImGuiEx::M3::M3Styles &m3Styles);

} // namespace Ime::UI
