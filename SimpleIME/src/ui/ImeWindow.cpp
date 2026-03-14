//
// Created by jamie on 2026/1/30.
//

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui/ImeWindow.h"

#include "ImeWnd.hpp"
#include "RE/GRectEx.h"
#include "RE/M/MenuCursor.h"
#include "WCharUtils.h"
#include "core/State.h"
#include "ime/ITextService.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/facade/base.h"
#include "utils/InputFocusAnchor.h"

namespace Ime
{

namespace
{
void DrawComposition(const CompositionInfo &compositionInfo)
{
    const auto             &documentText = compositionInfo.documentText;
    const std::wstring_view editorTextSv(documentText);

    bool drawStartToCaret = false;
    if (compositionInfo.caretPos > 0)
    {
        auto startToCaret = WCharUtils::ToString(editorTextSv.substr(0, compositionInfo.caretPos));
        if (startToCaret.empty())
        {
            // Conversion failed for a non-empty wstring segment; render a placeholder.
            startToCaret = "?";
        }
        ImGuiEx::M3::AlignedLabel(startToCaret);
        drawStartToCaret = true;
    }

    const auto &m3Styles = ImGuiEx::M3::Context::GetM3Styles();
    // caret
    if (fmod(ImGui::GetTime(), 1.2) <= 0.8) // NOLINT(*-magic-numbers)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        ImRect       caretRect;
        if (drawStartToCaret)
        {
            const auto lastItemMin = ImGui::GetItemRectMin();
            const auto lastItemMax = ImGui::GetItemRectMax();
            caretRect.Min          = ImVec2(lastItemMax.x, lastItemMin.y);
            caretRect.Max          = ImVec2(lastItemMax.x, lastItemMax.y);
        }
        else
        {
            const auto &lineHeight = m3Styles.GetLastText().currText.lineHeight;
            const float offsetY    = ImGuiEx::M3::HalfDiff(window->DC.CurrLineSize.y, lineHeight);
            caretRect.Min          = ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + offsetY);
            caretRect.Max          = ImVec2(caretRect.Min.x, caretRect.Min.y + lineHeight);
        }

        const auto caretColor = m3Styles.Colors()[ImGuiEx::M3::Spec::TextFieldCommon::CaretColor];
        window->DrawList->AddLine(caretRect.Min, caretRect.Max, ImGui::ColorConvertFloat4ToU32(caretColor), 1.0f);
    }

    if (compositionInfo.caretPos < documentText.size())
    {
        const auto caretToEnd = WCharUtils::ToString(editorTextSv.substr(compositionInfo.caretPos));
        if (!caretToEnd.empty())
        {
            ImGui::SameLine(0.F, 0.F);
            ImGuiEx::M3::AlignedLabel(caretToEnd);
        }
    }
}

void DrawCandidates(const CandidateUi &candidateUi)
{
    using size_type = CandidateUi::size_type;

    if (const auto candidateList = candidateUi.CandidateList(); !candidateList.empty())
    {
        ImGuiEx::M3::BeginChipGroup();

        ImGuiEx::M3::ChipConfiguration configuration{};
        configuration.colors = ImGuiEx::M3::Spec::ChipColors::Filter;

        size_type index   = 0;
        size_type clicked = candidateList.size();
        for (const auto &candidate : candidateList)
        {
            ImGui::PushID(static_cast<int>(index));
            configuration.selected = index == candidateUi.Selection();
            if (ImGuiEx::M3::Chip(candidate.c_str(), configuration))
            {
                clicked = index;
            }
            ImGui::SameLine();
            ImGui::PopID();
            index++;
        }
        if (clicked < candidateList.size())
        {
            ImeController::GetInstance()->CommitCandidate(static_cast<DWORD>(clicked));
        }
        ImGuiEx::M3::EndChipGroup();
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

auto UpdateImeWindowPos(Settings::WindowPosUpdatePolicy policy, ImVec2 &windowPos) -> void
{
    switch (policy)
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
 * @brief Ensures the IME window is fully contained within the viewport and avoids the input area.
 * @details
 * **Implementation Logic:**
 * Replaces manual clamping with ImGui internal `FindBestWindowPosForPopupEx`. This leverages
 * engine-native logic to calculate the optimal position relative to the `avoidRect`
 * (the text input area) while respecting viewport boundaries.
 */
void ClampWindowToViewport(Settings::WindowPosUpdatePolicy policy, ImVec2 &pos, const ImVec2 &size, ImGuiDir &lastAutoPosDir)
{
    const auto &viewport = ImGui::GetMainViewport();
    ImRect      avoidRect;
    if (policy == Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR)
    {
        avoidRect.Min = pos;
        avoidRect.Max = pos;
    }
    else
    {
        const auto &bounds = InputFocusAnchor::GetInstance().GetLastBounds();
        avoidRect.Min.x    = bounds.left;
        avoidRect.Min.y    = bounds.top;
        avoidRect.Max.x    = bounds.right;
        avoidRect.Max.y    = bounds.bottom;
    }
    const ImRect viewPortRect(viewport->Pos, viewport->Pos + viewport->Size);
    pos = ImGui::FindBestWindowPosForPopupEx(pos, size, &lastAutoPosDir, viewPortRect, avoidRect, ImGuiPopupPositionPolicy_ComboBox);
}
} // namespace

void ImeWindow::Draw(const CompositionInfo &compositionInfo, const CandidateUi &candidateUi, const Settings &settings)
{
    const auto &state = Core::State::GetInstance();
    if (state.ImeDisabled() || !state.IsImeInputting() /* || !state.HasAny(State::KEYBOARD_OPEN, State::IME_OPEN)*/)
    {
        ImGui::CloseCurrentPopup();
        return;
    }
    const auto currentFrame = ImGui::GetFrameCount();
    if (currentFrame > m_lastShowFrame + 1)
    {
        m_shouldRelayout = true;
        UpdateImeWindowPos(settings.input.posUpdatePolicy, m_imePos);
    }
    m_lastShowFrame = currentFrame;
    if (m_shouldRelayout)
    {
        m_shouldRelayout = false;
        ClampWindowToViewport(settings.input.posUpdatePolicy, m_imePos, m_imeSize, m_lastAutoPosDir);
        ImGui::SetNextWindowPos({m_imePos.x, m_imePos.y});
    }
    constexpr auto flags          = ImGuiEx::WindowFlags().NoDecoration().AlwaysAutoResize().NoFocusOnAppearing().NoSavedSettings().NoNav();
    const auto     mainStyleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_WindowPadding>({});

    // Why not use `BeginComboPopup`?
    // While the IME window's lifecycle resembles a Combo, `BeginComboPopup` is unsuitable because:
    // 1. **Focus/Closure Policy:** Popups automatically close on "click-outside," which conflicts
    // with IME persistence requirements during composition.
    // 2. **State Control:** IME visibility is strictly driven by `Core::State`, whereas Popups
    // rely on ImGui's internal `OpenPopup` stack, leading to state synchronization issues.
    if (ImGui::Begin("IME", nullptr, flags))
    {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

        ImGuiEx::M3::ListItemPlain([&] {
            DrawComposition(compositionInfo);
        });

        ImGuiEx::M3::Divider();

        ImGuiEx::M3::ListItemPlain([&] {
            DrawCandidates(candidateUi);
        });

        m_imeSize = ImGui::GetWindowSize();
    }
    ImGui::End();
    if (IsImeNeedRelayout())
    {
        m_shouldRelayout = true;
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
