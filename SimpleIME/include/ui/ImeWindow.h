//
// Created by jamie on 2026/1/30.
//

#pragma once

#include "Settings.h"
#include "imgui.h"

namespace ImGuiEx::M3
{
class M3Styles;
}

namespace Ime
{
class ImeWnd;

class ITextService;

class ImeWindow
{
    ImVec2        m_imeWindowSize = ImVec2(0, 0);
    ImVec2        m_imeWindowPos  = ImVec2(0, 0);
    ImeWnd       &m_imeWnd;
    ITextService *m_pTextService = nullptr;

public:
    explicit ImeWindow(ImeWnd &imeWnd, ITextService *pTextService) : m_imeWnd(imeWnd), m_pTextService(pTextService) {}

    void Draw(const Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles);

private:
    void DrawCompWindow(const ImGuiEx::M3::M3Styles &m3Styles) const;
    void DrawCandidateWindows(const ImGuiEx::M3::M3Styles &m3Styles) const;

    auto        IsImeNeedRelayout() const -> bool;
    static auto UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void;
    static auto UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void;
    /**
     * @brief Ensure the current ImGui window is fully within the viewport.
     */
    static void ClampWindowToViewport(const ImVec2 &windowSize, ImVec2 &windowPos);
};
}
