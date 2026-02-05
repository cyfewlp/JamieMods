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
class CandidateUi;
class TextEditor;

class ImeWindow
{
    ImVec2 m_imePos;
    ImVec2 m_imeSize;

public:
    void Draw(
        const TextEditor &textEditor, const CandidateUi &candidateUi, const Settings &settings,
        const ImGuiEx::M3::M3Styles &m3Styles
    );

private:
    [[nodiscard]] auto IsImeNeedRelayout() const -> bool;
};
} // namespace Ime
