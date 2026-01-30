//
// Created by jamie on 2026/1/30.
//

#pragma once

#include "Settings.h"
#include "common/config.h"
#include "common/imgui/Material3.h"
#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class ImeWnd;

class ITextService;

class ImeWindow
{
    ImVec2                 m_imeWindowSize = ImVec2(0, 0);
    ImVec2                 m_imeWindowPos  = ImVec2(0, 0);
    ImeWnd                &m_imeWnd;
    ITextService          *m_pTextService = nullptr;
    ImGuiEx::M3::M3Styles &m_styles;

public:
    explicit ImeWindow(ImeWnd &imeWnd, ITextService *pTextService, ImGuiEx::M3::M3Styles &m3Styles)
        : m_imeWnd(imeWnd), m_pTextService(pTextService), m_styles(m3Styles)
    {
    }

    void Draw(const Settings &settings);

private:
    void DrawCompWindow() const;
    void DrawCandidateWindows() const;

    auto        IsImeNeedRelayout() const -> bool;
    static auto UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void;
    static auto UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void;
    /**
     * @brief Ensure the current ImGui window is fully within the viewport.
     */
    static void ClampWindowToViewport(const ImVec2 &windowSize, ImVec2 &windowPos);
};
}
}
