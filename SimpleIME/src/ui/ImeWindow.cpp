//
// Created by jamie on 2026/1/30.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui/ImeWindow.h"

#include "ImeWnd.hpp"
#include "RE/M/MenuCursor.h"
#include "common/WCharUtils.h"
#include "common/imgui/ImGuiEx.h"
#include "configs/CustomMessage.h"
#include "core/State.h"
#include "ime/ITextService.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/TaskQueue.h"
#include "utils/FocusGFxCharacterInfo.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
void ImeWindow::Draw(const Settings &settings)
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
        UpdateImeWindowPos(settings, m_imeWindowPos);
    }
    if (shouldRelayout)
    {
        shouldRelayout = false;
        ClampWindowToViewport(m_imeWindowSize, m_imeWindowPos);
        ImGui::SetNextWindowPos(m_imeWindowPos);
    }

    constexpr auto flags =
        ImGuiEx::WindowFlags().NoDecoration().AlwaysAutoResize().NoFocusOnAppearing().NoSavedSettings().NoNav();
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Push(ImGuiEx::StyleHolder::WindowPadding({}))
        .Push(ImGuiEx::ColorHolder::Text(m_styles.colors[ImGuiEx::M3::SurfaceToken::primary]))
        .Push(ImGuiEx::ColorHolder::WindowBg(m_styles.colors[ImGuiEx::M3::SurfaceToken::surfaceContainerLow]))
        .Push(ImGuiEx::ColorHolder::Separator(m_styles.colors[ImGuiEx::M3::SurfaceToken::outlineVariant]));

    ImGui::PushFont(nullptr, m_styles.GetLargeText().fontSize);
    if (ImGui::Begin("IME", nullptr, flags))
    {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        if (state.Has(Core::State::IN_COMPOSING))
        {
            DrawCompWindow();
        }

        ImGui::Separator();
        // render ime status window: language,
        if (state.Has(Core::State::IN_CAND_CHOOSING))
        {
            DrawCandidateWindows();
        }
        m_imeWindowSize = ImGui::GetWindowSize();
    }
    ImGui::End();
    ImGui::PopFont();
    if (IsImeNeedRelayout())
    {
        shouldRelayout = true;
    }
}

void ImeWindow::DrawCompWindow() const
{
    static float CursorAnim = 0.F;

    CursorAnim += ImGui::GetIO().DeltaTime;

    const auto &textEditor = m_pTextService->GetTextEditor();

    const auto &editorText = textEditor.GetText();
    LONG        acpStart   = 0;
    LONG        acpEnd     = 0;
    textEditor.GetSelection(&acpStart, &acpEnd);

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
    styleGuard.Push(ImGuiEx::StyleHolder::FramePadding({0, m_styles[ImGuiEx::M3::Spacing::S]}));
    ImGui::SameLine(0, m_styles[ImGuiEx::M3::Spacing::M]);
    ImGui::AlignTextToFramePadding();

    bool success;
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
    ImGui::SameLine(0, 1.f);
    if (fmodf(CursorAnim, 1.2F) <= 0.8F)
    {
        ImVec2 const cursorScreenPos = ImGui::GetCursorScreenPos();
        ImVec2 const min(cursorScreenPos.x, cursorScreenPos.y + 0.5f + m_styles[ImGuiEx::M3::Spacing::S]);
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

void ImeWindow::DrawCandidateWindows() const
{
    using namespace ImGuiEx::M3;
    const auto &candidateUi = m_pTextService->GetCandidateUi();
    if (const auto candidateList = candidateUi.CandidateList(); !candidateList.empty())
    {
        DWORD index   = 0;
        DWORD clicked = candidateList.size();

        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Push(ImGuiEx::StyleHolder::ItemSpacing({0, 0}))
            .Push(ImGuiEx::StyleHolder::FramePadding({m_styles[Spacing::M], m_styles[Spacing::S]}))
            .Push(ImGuiEx::ColorHolder::Text(m_styles.colors[ContentToken::onSurface]))
            .Push(ImGuiEx::ColorHolder::Button({0, 0, 0, 0}))
            .Push(
                ImGuiEx::ColorHolder::ButtonHovered(
                    m_styles.colors[SurfaceToken::surfaceContainer].Hovered(m_styles.colors[ContentToken::onSurface])
                )
            )
            .Push(
                ImGuiEx::ColorHolder::ButtonActive(
                    m_styles.colors[SurfaceToken::surfaceContainer].Pressed(m_styles.colors[ContentToken::onSurface])
                )
            );

        for (const auto &candidate : candidateList)
        {
            ImGui::PushID(static_cast<int>(index));
            ImGuiEx::StyleGuard styleGuard1;
            if (index == candidateUi.Selection())
            {
                styleGuard1.Push(ImGuiEx::ColorHolder::Button(m_styles.colors[SurfaceToken::primaryContainer]))
                    .Push(ImGuiEx::ColorHolder::Text(m_styles.colors[ContentToken::onPrimaryContainer]))
                    .Push(
                        ImGuiEx::ColorHolder::ButtonHovered(m_styles.colors[SurfaceToken::primaryContainer].Hovered(
                            m_styles.colors[ContentToken::onPrimaryContainer]
                        ))
                    )
                    .Push(
                        ImGuiEx::ColorHolder::ButtonActive(m_styles.colors[SurfaceToken::primaryContainer].Pressed(
                            m_styles.colors[ContentToken::onPrimaryContainer]
                        ))
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
            TaskQueue::GetInstance().AddImeThreadTask([this, clicked] {
                m_pTextService->CommitCandidate(m_imeWnd.GetHWND(), clicked);
            });
            PostMessageA(m_imeWnd.GetHWND(), CM_EXECUTE_TASK, 0, 0);
        }
    }
}

// FIXME: may also check the min position
auto ImeWindow::IsImeNeedRelayout() const -> bool
{
    const auto &viewport    = ImGui::GetMainViewport();
    const auto  rectMax     = m_imeWindowPos + m_imeWindowSize;
    const auto  viewportMax = viewport->Size + viewport->Pos;
    return rectMax.x > viewportMax.x || rectMax.y > viewportMax.y;
}

auto ImeWindow::UpdateImeWindowPos(const Settings &settings, ImVec2 &windowPos) -> void
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
            FocusGFxCharacterInfo::GetInstance().UpdateCaretCharBoundaries();
            UpdateImeWindowPosByCaret(windowPos);
            break;
        }
        default:;
    }
}

auto ImeWindow::UpdateImeWindowPosByCaret(ImVec2 &windowPos) -> void
{
    const auto &instance       = FocusGFxCharacterInfo::GetInstance();
    const auto &charBoundaries = instance.CharBoundaries();

    windowPos.x = charBoundaries.left;
    windowPos.y = charBoundaries.bottom;
}

void ImeWindow::ClampWindowToViewport(const ImVec2 &windowSize, ImVec2 &windowPos)
{
    const auto &viewport   = ImGui::GetMainViewport();
    const float viewRight  = viewport->Pos.x + viewport->Size.x;
    const float viewBottom = viewport->Pos.y + viewport->Size.y;
    windowPos.x            = std::max(windowPos.x, viewport->Pos.x);
    windowPos.y            = std::max(windowPos.y, viewport->Pos.y);

    if (windowSize.x < viewport->Size.x && windowPos.x + windowSize.x > viewRight)
    {
        windowPos.x = viewRight - windowSize.x;
    }
    if (windowSize.y < viewport->Size.y && windowPos.y + windowSize.y > viewBottom)
    {
        windowPos.y = viewBottom - windowSize.y;
    }

    const ImVec2 min            = windowPos;
    const ImVec2 max            = {windowPos.x + windowSize.x, windowPos.y + windowSize.y};
    const auto  &instance       = FocusGFxCharacterInfo::GetInstance();
    const auto  &charBoundaries = instance.CharBoundaries();
    if (charBoundaries.top < max.y && charBoundaries.bottom > min.y && charBoundaries.left < max.x &&
        charBoundaries.right > min.x) // is overlaps?
    {
        // Move the window above the boundary
        if (const float newY = charBoundaries.top - windowSize.y; newY >= viewport->Pos.y)
        {
            windowPos.y = newY;
        }
    }
}
}
}
