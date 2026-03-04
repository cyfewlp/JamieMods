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
class CompositionInfo;
class CandidateUi;

class ImeWindow
{
    ImVec2   m_imePos;
    ImVec2   m_imeSize;
    ImGuiDir m_lastAutoPosDir = ImGuiDir_Down;
    int      m_lastShowFrame  = -1;
    bool     m_shouldRelayout = true;

public:
    void Draw(const CompositionInfo &compositionInfo, const CandidateUi &candidateUi, const Settings &settings);

private:
    [[nodiscard]] auto IsImeNeedRelayout() const -> bool;
};
} // namespace Ime
