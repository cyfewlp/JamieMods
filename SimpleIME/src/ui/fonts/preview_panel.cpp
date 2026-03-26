//
// Created by jamie on 2026/2/8.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui/fonts/preview_panel.h"

#include "WCharUtils.h"
#include "fonts/FontManager.h"
#include "i18n/Translator.h"
#include "i18n/translator_manager.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/facade/base.h"
#include "imguiex/m3/spec/layout.h"
#include "ui/fonts/ImFontWrap.h"

namespace Ime::UI
{
// ReSharper disable All
static constexpr auto PREVIEW_TEXT = {
    R"(!@#$%^&*()_+-=[]{}|;':",.<>?/)",
    R"(-- Unicode & Fallback --)",
    R"(Latín: áéíóú ñ  |  FullWidth: ＡＢＣ１２３)",
    R"(CJK: 繁體中文测试 / 简体中文测试 / 日本語 / 한국어)",
    R"(-- Emoji & Variation --)",
    R"(Icons: 🥰💀✌︎🌴🐢🐐🍄🍻👑📸😬👀🚨🏡)",
    R"(New: 🐦‍🔥 🍋‍🟩 🍄‍🟫 🙂‍↕️ 🙂‍↔️)",
    R"(-- Skyrim Immersion --)",
    R"(Dovah: Dovahkiin, naal ok zin los vahriin!)",
    R"("I used to be an adventurer like you...")",
};

// ReSharper restore All

namespace
{

struct StatusBar
{
    std::string_view icon;
    std::string_view text;
};

/**
 * @param selectedIndex current selected FontInfo index. will be set if select a new.
 * @return is select a new row.
 */
auto FontsTable(Fonts::FontInfo::Index &selectedIndex, const std::vector<Fonts::FontInfo> &fontInfos) -> bool
{
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(fontInfos.size()));
    bool selectedNew = false;
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
        {
            const auto &fontInfo = fontInfos[static_cast<size_t>(row)];
            ImGui::PushID(row);

            const bool selected = selectedIndex == fontInfo.GetIndex();
            const bool clicked  = ImGuiEx::M3::MenuItem(fontInfo.GetName(), selected);
            if (clicked && !selected)
            {
                selectedNew   = true;
                selectedIndex = fontInfo.GetIndex();
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
        }
    }

    return selectedNew;
}

void DrawStatusBar(const StatusBar &statusBar)
{
    ImGuiEx::M3::SmallIcon(statusBar.icon);
    ImGui::SameLine(0.0F, 0.0F);
    ImGuiEx::M3::AlignedLabel(statusBar.text);
    ImGuiEx::M3::Divider();
}

/**
 * contains an "apply" button to ensure if add font to @c FontBuilder
 * @return true if click the "apply" button, false otherwise.
 */
auto PreviewPanel(const ImGuiEx::M3::M3Styles &m3Styles, ImFontWrap &displayFont) -> bool
{
    ImGui::PushFont(displayFont.UnsafeGetFont(), 0);
    for (const auto &text : PREVIEW_TEXT)
    {
        ImGuiEx::M3::TextUnformatted<ImGuiEx::M3::Spec::TextRole::BodyLarge>(text);
    }
    ImGui::PopFont();

    if (!displayFont.IsCommittable())
    {
        return false;
    }

    // Absolute layout: the FAB should be placed in the bottom right corner of the preview panel,
    // regardless of the content size.
    const auto cursorPos = ImGui::GetCursorScreenPos();
    const auto avail     = ImGui::GetContentRegionAvail();

    constexpr auto FabSize       = M3Spec::FabSizing<ImGuiEx::M3::Spec::SizeTips::MEDIUM>::ContainerHeight;
    const auto     scaledFabSize = m3Styles.GetPixels(FabSize);
    ImGui::SetCursorScreenPos(cursorPos + avail - ImVec2(scaledFabSize, scaledFabSize - ImGui::GetScrollY()));
    const bool clicked = ImGuiEx::M3::Fab(ICON_PLUS, ImGuiEx::M3::Spec::FabColors::TonalPrimary);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Add"));
    return clicked;
}

} // namespace

void FontPreviewPanel::DrawFontsView(const std::vector<Fonts::FontInfo> &fontInfos)
{
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    const bool edited = ImGuiEx::M3::SearchBar("Filter", m_textFilter.InputBuf, IM_COUNTOF(m_textFilter.InputBuf), {.icon = ICON_SEARCH});
    ImGui::PopItemFlag();
    if (edited)
    {
        m_searchDebounceTimer.Poke();
    }

    const auto &displayFontInfos = m_displayFontInfos.empty() ? fontInfos : m_displayFontInfos;
    if (FontsTable(m_interactState.selectedIndex, displayFontInfos))
    {
        m_previewDebounceTimer.Poke();
        m_state = State::DEBOUNCING;
    }

    if (m_searchDebounceTimer.Check())
    {
        m_textFilter.Build();
        UpdateDisplayFontInfos(fontInfos);
    }

    if (m_previewDebounceTimer.Check())
    {
        m_state           = State::EMPTY;
        const auto fontId = static_cast<size_t>(m_interactState.selectedIndex);
        if (fontId >= fontInfos.size())
        {
            m_previewDebounceTimer.Reset();
        }
        else
        {
            const auto &fontInfo = fontInfos[fontId];
            const auto  filePath = GetFontFilePath(fontInfo);
            m_imFont.Cleanup();
            if (!filePath.empty())
            {
                PreviewFont(fontInfo.GetName(), WCharUtils::ToString(filePath));
                m_state = State::PREVIEWING;
            }
            else
            {
                m_state = State::NOT_SUPPORTED_FONTS;
            }
        }
    }
}

//! Submit two child windows.
//! Child1: Fonts info, auto resize along X-axis, fixed height, with border.
//! Child2: Font preview, auto extend along X-axis, fill remaining height. contains an "apply" button to ensure if add font to @c FontBuilder
void FontPreviewPanel::Draw(FontBuilder &fontBuilder)
{
    if (ImGui::BeginChild("FontsView", {}, ImGuiEx::ChildFlags().AutoResizeX()))
    {
        DrawFontsView(fontBuilder.GetFontManager().GetFontInfoList());
    }
    ImGui::EndChild();
}

void FontPreviewPanel::DrawPreviewPanel(FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles)
{
    if (m_state == State::NOT_SUPPORTED_FONTS)
    {
        return;
    }

    StatusBar statusBar;
    switch (m_state)
    {
        case State::DEBOUNCING:
            statusBar.icon = ICON_REFRESH_CW;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.Debouncing");
            break;
        case State::PREVIEW_BUILDER_FONT:
            statusBar.icon = ICON_EYE;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.BuilderFont");
            break;
        case State::NOT_SELECTED_FONT: {
            statusBar.icon = ICON_CIRCLE_ALERT;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.NotSelectedFont");
            break;
        }
        case State::NOT_SUPPORTED_FONTS: {
            statusBar.icon = ICON_FILE_QUESTION_MARK;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.NotSupportedFont");
            break;
        }
        case State::PREVIEWING:
            statusBar.icon = ICON_FILE_CHECK;
            statusBar.text = m_imFont.GetFontPathOr(0);
            break;
        default:
            statusBar.icon = ICON_FILE_QUESTION_MARK;
            break;
    }

    const auto margin     = m3Styles.GetPixels(M3Spec::Layout::ExtraLarge::Margin);
    const auto styleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_ChildBg>(m3Styles.Colors()[M3Spec::ColorRole::surfaceContainerLowest]);
    if (ImGui::BeginChild("PreviewPanel", {-margin, 0.F}, ImGuiEx::ChildFlags().AutoResizeY()))
    {
        DrawStatusBar(statusBar);
        const auto textPadding = m3Styles.GetPixels(M3Spec::TextParagraph::PaddingX);
        ImGui::Indent(textPadding);
        auto clicked = PreviewPanel(m3Styles, m_imFont);
        ImGui::Unindent(textPadding);
        if (clicked)
        {
            if (fontBuilder.AddFont(m_interactState.selectedIndex, m_imFont))
            {
                Cleanup();
            }
        }
    }
    ImGui::EndChild();
}

void FontPreviewPanel::UpdateDisplayFontInfos(const std::vector<Fonts::FontInfo> &sourceList)
{
    m_displayFontInfos.clear();
    for (const auto &fontInfo : sourceList)
    {
        if (m_textFilter.PassFilter(fontInfo.GetName().c_str()))
        {
            m_displayFontInfos.push_back(fontInfo);
        }
    }
}

void FontPreviewPanel::PreviewFont(const std::string &fontName, const std::string &fontPath)
{
    auto *imFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.c_str());
    m_imFont     = ImFontWrap(imFont, fontName, fontPath, true);
}

void FontPreviewPanel::Cleanup()
{
    m_imFont.Cleanup();
    m_state = State::NOT_SELECTED_FONT;
}

} // namespace Ime::UI
