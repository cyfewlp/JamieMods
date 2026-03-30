//
// Created by jamie on 2026/3/26.
//

#pragma once

#include "imgui.h"

#include <d3d11.h>
#include <string>
#include <vector>

namespace ImGuiEx
{

void Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context);

/**
 * @brief Add the primary font
 * Supporting the passing of multiple fonts as fallback fonts, which will be automatically merged and set as the default font
 * If all the provide font ivalid or load failed, will fallback to ImGui embedded default font.
 * @param fontsPathList the primary font and fallback fonts file path list
 * @param fallbackFontsPathList [optional] if the primary font load failed, will fallback to these fonts in order. If the primary font load
 * successfully, these fonts will be ignored. Can be empty.
 * @return the ImFont pointer.
 */
auto AddPrimaryFont(const std::vector<std::string> &fontsPathList, const std::vector<std::string> &fallbackFontsPathList) -> ImFont *;

/**
 * @brief Add a seprate font
 * Add a seprate font which is not merged to primary font, can be used for ICON font or other special usage.
 */
[[nodiscard]] auto AddFont(const std::string &filePath) -> ImFont *;

void NewFrame();

void Render();

void EndFrame();

void Shutdown();

} // namespace ImGuiEx
