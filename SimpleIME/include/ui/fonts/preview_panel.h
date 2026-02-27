//
// Created by jamie on 2026/1/16.
//

#pragma once

#include "ui/DebounceTimer.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/ImFontWrap.h"

#include <chrono>
#include <cstdint>
#include <imgui.h>
#include <string>
#include <vector>

namespace ImGuiEx::M3
{
class M3Styles;
}

struct ImGuiTextFilter;

namespace Ime::UI
{
class FontPreviewPanel
{
    ImGuiTextFilter       m_textFilter;
    ImFontWrap            m_imFont;
    std::vector<FontInfo> m_displayFontInfos;
    DebounceTimer         m_previewDebounceTimer{std::chrono::milliseconds{300}};
    DebounceTimer         m_searchDebounceTimer{std::chrono::milliseconds{200}};

    enum class State : int8_t
    {
        EMPTY = 0,
        PREVIEWING,
        DEBOUNCING,
        NOT_SUPPORTED_FONTS,
        NOT_SELECTED_FONT,
        PREVIEW_BUILDER_FONT,
    };

    State m_state = State::EMPTY;

public:
    FontPreviewPanel() = default;

    struct InteractState
    {
        bool didInteract   = false;
        int  selectedIndex = -1;
    };

    void Draw(FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles);

private:
    void DrawFontsView(const std::vector<FontInfo> &fontInfos);
    void UpdateDisplayFontInfos(const std::vector<FontInfo> &sourceList);

public:
    [[nodiscard]] auto IsPreviewing() const -> bool { return m_state == State::PREVIEWING; }

private:
    void PreviewFont(const std::string &fontName, const std::string &fontPath);

public:
    void PreviewFont(const ImFontWrap &imFont)
    {
        m_imFont = imFont;
        m_state  = State::PREVIEW_BUILDER_FONT;
    }

    void Cleanup();

    [[nodiscard]] auto GetInteractState() const -> const InteractState & { return m_interactState; }

    [[nodiscard]] auto GetImFont() const -> const ImFontWrap & { return m_imFont; }

    [[nodiscard]] auto GetImFont() -> ImFontWrap & { return m_imFont; }

private:
    InteractState m_interactState;
};
} // namespace Ime::UI
