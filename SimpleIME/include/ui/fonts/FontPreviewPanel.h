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
    ImFontWrap            m_imFont{};
    DebounceTimer         m_previewDebounceTimer{300ms};
    DebounceTimer         m_searchDebounceTimer{200ms};
    std::vector<FontInfo> m_displayFontInfos;
    ImGuiTextFilter       m_textFilter;

public:
    struct InteractState
    {
        bool interact      = false;
        int  selectedIndex = -1;
    };

    void DrawFontsView(const std::vector<FontInfo> &fontInfos);
    void DrawFontsPreviewView(const Translation &translation) const;

private:
    void DrawSearchBox(const std::vector<FontInfo> &fontInfos);
    void DrawFontsTable(const std::vector<FontInfo> &fontInfos);
    void UpdateDisplayFontInfos(const std::vector<FontInfo> &sourceList);

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