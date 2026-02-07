//
// Created by jamie on 2026/1/30.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui/ImeWindow.h"

#include "ImeWnd.hpp"
#include "RE/GRectEx.h"
#include "RE/M/MenuCursor.h"
#include "common/WCharUtils.h"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/imguiex_enum_wrap.h"
#include "core/State.h"
#include "ime/CandidateUi.h"
#include "ime/ImeController.h"
#include "ime/TextEditor.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "utils/InputFocusAnchor.h"

namespace Ime
{

namespace
{
void DrawComposition(const TextEditor &editor, const ImGuiEx::M3::M3Styles &m3Styles)
{
    const auto &editorText = editor.GetText();
    LONG        acpStart   = 0;
    LONG        acpEnd     = 0;
    editor.GetSelection(&acpStart, &acpEnd);

    const size_t textSize = editorText.size();
    acpStart              = std::max(0L, acpStart);
    acpEnd                = std::max(0L, acpEnd);

    acpStart = std::min(static_cast<long>(textSize), acpStart);
    acpEnd   = std::min(static_cast<long>(textSize), acpEnd);

    if (acpEnd < acpStart)
    {
        std::swap(acpStart, acpEnd);
    }

    // use frame padding set line height
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Style_FramePadding({0, m3Styles[ImGuiEx::M3::Spacing::S]});
    ImGui::SameLine(0, m3Styles[ImGuiEx::M3::Spacing::M]);
    ImGui::AlignTextToFramePadding();

    bool success = false;
    if (acpStart > 0)
    {
        std::string startToCaret;
        success = WCharUtils::ToString(editorText.c_str(), acpStart, startToCaret);
        if (success)
        {
            ImGui::TextUnformatted(startToCaret.c_str());
        }
    }

    // caret
    ImGui::SameLine(0, 1.F);
    if (fmod(ImGui::GetTime(), 1.2) <= 0.8) // NOLINT(*-magic-numbers)
    {
        ImVec2 const cursorScreenPos = ImGui::GetCursorScreenPos();
        ImVec2 const min(cursorScreenPos.x, cursorScreenPos.y + 0.5f + m3Styles[ImGuiEx::M3::Spacing::S]);
        ImGui::GetWindowDrawList()->AddLine(
            min, ImVec2(min.x, min.y + ImGui::GetFontSize() - 1.5f), ImGui::GetColorU32(ImGuiCol_InputTextCursor), 1.0f
        );
    }

    std::string caretToEnd;
    const auto  subFromCaret = editorText.substr(acpStart);
    success                  = false;
    if (!subFromCaret.empty())
    {
        success = WCharUtils::ToString(subFromCaret.c_str(), static_cast<int>(subFromCaret.size()), caretToEnd);
        if (success)
        {
            ImGui::TextUnformatted(caretToEnd.c_str());
        }
    }

    if (!success)
    {
        ImGui::NewLine();
    }
}

void DrawCandidates(const CandidateUi &candidateUi, const ImGuiEx::M3::M3Styles &m3Styles)
{
    using Spacing      = ImGuiEx::M3::Spacing;
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    if (const auto candidateList = candidateUi.CandidateList(); !candidateList.empty())
    {
        DWORD index   = 0;
        DWORD clicked = candidateList.size();

        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style_ItemSpacing({0, 0})
            .Style_FramePadding({m3Styles[Spacing::M], m3Styles[Spacing::S]})
            .Color_Text(m3Styles.Colors().at(ContentToken::onSurface))
            .Color_Button({0, 0, 0, 0})
            .Color_ButtonHovered(m3Styles.Colors().Hovered(SurfaceToken::surfaceContainer, ContentToken::onSurface))
            .Color_ButtonActive(m3Styles.Colors().Pressed(SurfaceToken::surfaceContainer, ContentToken::onSurface));

        for (const auto &candidate : candidateList)
        {
            ImGui::PushID(static_cast<int>(index));
            ImGuiEx::StyleGuard styleGuard1;
            if (index == candidateUi.Selection())
            {
                styleGuard1.Color_Button(m3Styles.Colors()[SurfaceToken::primaryContainer])
                    .Color_Text(m3Styles.Colors()[ContentToken::onPrimaryContainer])
                    .Color_ButtonHovered(
                        m3Styles.Colors().Hovered(SurfaceToken::primaryContainer, ContentToken::onPrimaryContainer)
                    )
                    .Color_ButtonActive(
                        m3Styles.Colors().Pressed(SurfaceToken::primaryContainer, ContentToken::onPrimaryContainer)
                    );
            }

            if (ImGui::Button(candidate.c_str()))
            {
                clicked = index;
            }
            ImGui::SameLine();
            ImGui::PopID();
            index++;
        }
        if (clicked < candidateList.size())
        {
            ImeController::GetInstance()->CommitCandidate(clicked);
        }
    }
}

auto UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void
{
    auto &instance = InputFocusAnchor::GetInstance();
    instance.ComputeScreenMetrics();
    const auto &bounds = instance.GetLastBounds();

    windowPos.x = bounds.left;
    windowPos.y = bounds.bottom;
}

auto UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void
{
    switch (settings.input.posUpdatePolicy)
    {
        case Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR:
            if (const auto *cursor = RE::MenuCursor::GetSingleton(); cursor != nullptr)
            {
                windowPos.x = cursor->cursorPosX;
                windowPos.y = cursor->cursorPosY;
            }
            break;
        case Settings::WindowPosUpdatePolicy::BASED_ON_CARET: {
            UpdateImeWindowPosByCaret(windowPos);
            break;
        }
        default:;
    }
}

/**
 * @brief Ensure the current ImGui window is fully within the viewport.
 */
void ClampWindowToViewport(ImVec2 &pos, const ImVec2 &size)
{
    const auto &viewport = ImGui::GetMainViewport();

    const RE::GPointF max = {.x = pos.x + size.x, .y = pos.y + size.y};

    auto maxX = viewport->Pos.x + viewport->Size.x;
    auto maxY = viewport->Pos.y + viewport->Size.y;
    if (size.x < viewport->Size.x)
    {
        maxX -= size.x;
    }
    if (size.y < viewport->Size.y)
    {
        maxY -= -size.y;
    }
    pos.x = std::clamp(pos.x, viewport->Pos.x, maxX);
    pos.y = std::clamp(pos.y, viewport->Pos.y, maxY);

    if (const auto &bounds = InputFocusAnchor::GetInstance().GetLastBounds();
        Intersects(bounds, RE::GRectF{.left = pos.x, .top = pos.y, .right = max.x, .bottom = max.y})) // is overlaps?
    {
        // Move the window above the boundary
        if (const float newY = bounds.top - size.y; newY >= viewport->Pos.y)
        {
            pos.y = newY;
        }
    }
}
} // namespace

void ImeWindow::Draw(
    const TextEditor &textEditor, const CandidateUi &candidateUi, const Settings &settings,
    const ImGuiEx::M3::M3Styles &m3Styles
)
{
    static bool shouldRelayout = true;
    static bool imeAppearing   = true;
    const auto &state          = Core::State::GetInstance();
    if (state.ImeDisabled() || !state.IsImeInputting() /* || !state.HasAny(State::KEYBOARD_OPEN, State::IME_OPEN)*/)
    {
        imeAppearing   = true;
        shouldRelayout = true;
        ImGui::CloseCurrentPopup();
        return;
    }
    if (imeAppearing)
    {
        imeAppearing = false;
        UpdateImeWindowPos(settings, m_imePos);
    }
    if (shouldRelayout)
    {
        shouldRelayout = false;
        ClampWindowToViewport(m_imePos, m_imeSize);
        ImGui::SetNextWindowPos({m_imePos.x, m_imePos.y});
    }

    constexpr auto flags =
        ImGuiEx::WindowFlags().NoDecoration().AlwaysAutoResize().NoFocusOnAppearing().NoSavedSettings().NoNav();
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Style_WindowPadding({})
        .Color_Text(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::primary))
        .Color_WindowBg(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::surfaceContainerLow))
        .Color_Separator(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::outlineVariant));

    ImGui::PushFont(nullptr, m3Styles.TitleText().fontSize);
    if (ImGui::Begin("IME", nullptr, flags))
    {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        if (state.Has(Core::State::IN_COMPOSING))
        {
            DrawComposition(textEditor, m3Styles);
        }

        ImGui::Separator();
        // render ime status window: language,
        if (state.Has(Core::State::IN_CAND_CHOOSING))
        {
            DrawCandidates(candidateUi, m3Styles);
        }
        m_imeSize = ImGui::GetWindowSize();
    }
    ImGui::End();
    ImGui::PopFont();
    if (IsImeNeedRelayout())
    {
        shouldRelayout = true;
    }
}

auto ImeWindow::IsImeNeedRelayout() const -> bool
{
    const auto &viewport    = ImGui::GetMainViewport();
    const auto  rectMax     = m_imePos + m_imeSize;
    const auto  viewportMax = viewport->Size + viewport->Pos;
    return rectMax.x > viewportMax.x || rectMax.y > viewportMax.y;
}
} // namespace Ime
