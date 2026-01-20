//
// Created by jamie on 2026/1/16.
//

#pragma once

#include "common/config.h"
#include "ui/DebounceTimer.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/ImFontWrap.h"

#include <chrono>

struct ImGuiTextFilter;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class FontPreviewPanel
{
    ImFontWrap      m_imFont{nullptr, "", ""};
    DebounceTimer   m_debounceTimer{300ms};
    ImGuiTextFilter m_filter = {};

public:
    struct InteractState
    {
        bool interact      = false;
        int  selectedIndex = -1;
    };

    void DrawFontsView(FontBuilder &fontBuilder);
    void DrawFontsPreviewView() const;

private:
    void DrawSearchBox();
    void DrawFontsTable(FontBuilder &fontBuilder);

public:
    bool IsWaitingPreview() const
    {
        return m_interactState.selectedIndex >= 0;
    }

    bool IsWaitingCommit() const
    {
        return m_imFont.IsCommittable();
    }

    void PreviewFont(const std::string &fontName, const std::string &fontPath);

    void PreviewFont(const ImFontWrap &imFont)
    {
        m_imFont = imFont;
    }

    void Cleanup();

    auto GetInteractState() const -> const InteractState &
    {
        return m_interactState;
    }

    [[nodiscard]] auto GetImFont() const -> const ImFontWrap &
    {
        return m_imFont;
    }

    [[nodiscard]] auto GetImFont() -> ImFontWrap &
    {
        return m_imFont;
    }

private:
    InteractState m_interactState;
};
}
}