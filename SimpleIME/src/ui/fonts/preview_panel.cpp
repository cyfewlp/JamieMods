//
// Created by jamie on 2026/1/15.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui/fonts/FontPreviewPanel.h"

#include "common/i18n/Translator.h"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"
#include "i18n/TranslatorHolder.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontManager.h"
#include "ui/fonts/ImFontWrap.h"

namespace Ime
{
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

void FontPreviewPanel::DrawFontsView(const std::vector<FontInfo> &fontInfos, const ImGuiEx::M3::M3Styles &m3Styles)
{
    DrawSearchBox(fontInfos, m3Styles);
    DrawFontsTable(fontInfos, m3Styles);
}

void FontPreviewPanel::DrawFontsPreviewView(const ImGuiEx::M3::M3Styles &m3Styles) const
{
    DrawStatusBar(m3Styles);

    ImGui::Separator();
    ImGui::Dummy({0, m3Styles[ImGuiEx::M3::Spacing::L]});

    if (m_state == State::NOT_SUPPORTED_FONTS)
    {
        return;
    }

    // ReSharper disable All
    ImGuiEx::FontScope previewFont;
    if (m_imFont)
    {
        previewFont = ImGuiEx::FontScope(m_imFont.UnsafeGetFont());
    }
    // ReSharper restore All

    for (const auto &text : PREVIEW_TEXT)
    {
        ImGui::Text("%s", text);
    }
}

void FontPreviewPanel::DrawStatusBar(const ImGuiEx::M3::M3Styles &m3Styles) const
{
    std::string_view icon = "";
    std::string_view msg  = "";
    switch (m_state)
    {
        case State::DEBOUNCING:
            icon = ICON_MD_REFRESH;
            msg  = Translate("Settings.FontBuilder.PreviewPanel.Debouncing");
            break;
        case State::PREVIEW_BUILDER_FONT:
            icon = ICON_MD_EYE;
            msg  = Translate("Settings.FontBuilder.PreviewPanel.BuilderFont");
            break;
        case State::NOT_SELECTED_FONT: {
            icon = ICON_FA_CIRCLE_INFO;
            msg  = Translate("Settings.FontBuilder.PreviewPanel.NotSelectedFont");
            break;
        }
        case State::NOT_SUPPORTED_FONTS: {
            icon = ICON_FA_CIRCLE_EXCLAMATION;
            msg  = Translate("Settings.FontBuilder.PreviewPanel.NotSupportedFont");
            break;
        }
        case State::PREVIEWING: {
            icon = ICON_FA_FILE;
            msg  = m_imFont.GetFontPathOr(0);
            break;
        }
        default:
    }

    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Color_Text(m3Styles.Colors().at(ImGuiEx::M3::ContentToken::onSecondaryContainer));
        if (!icon.empty())
        {
            ImGui::Text("%s", icon.data());
            ImGui::SameLine(0, m3Styles[ImGuiEx::M3::Spacing::L]);
        }
        ImGui::TextWrapped("%s", msg.data());
    }
}

void FontPreviewPanel::DrawSearchBox(const std::vector<FontInfo> &fontInfos, const ImGuiEx::M3::M3Styles &m3Styles)
{
    using Spacing      = ImGuiEx::M3::Spacing;
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    drawList->ChannelsSetCurrent(1);
    ImGui::PushFont(nullptr, m3Styles.TitleText().fontSize);
    ImGui::BeginGroup();
    {
        ImGui::SetNextItemWidth(-m3Styles[Spacing::Double_L] - m3Styles.TitleText().fontSize);
        ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style_FramePadding({m3Styles[Spacing::L], m3Styles[Spacing::L]})
            .Color_Text(m3Styles.Colors().at(ContentToken::onSurfaceVariant))
            .Color_FrameBg({0, 0, 0, 0});
        if (ImGui::InputTextWithHint(
                "##Filter",
                "search by name",
                m_textFilter.InputBuf,
                IM_COUNTOF(m_textFilter.InputBuf),
                ImGuiInputTextFlags_EscapeClearsAll
            ))
        {
            m_searchDebounceTimer.Poke();
        }
        ImGui::PopItemFlag();
        ImGui::SameLine(0, m3Styles[Spacing::L]);
        ImGui::Text(ICON_OCT_SEARCH);
    }
    ImGui::EndGroup();
    ImGui::PopFont();

    if (m_searchDebounceTimer.Check())
    {
        m_textFilter.Build();
        UpdateDisplayFontInfos(fontInfos);
    }

    const auto rectMin = ImGui::GetItemRectMin();
    const auto rectMax = ImGui::GetItemRectMax();
    drawList->ChannelsSetCurrent(0);
    drawList->AddRectFilled(
        rectMin,
        {rectMax.x + m3Styles[Spacing::L], rectMax.y},
        ImGui::ColorConvertFloat4ToU32(m3Styles.Colors().at(SurfaceToken::surfaceContainerHigh)),
        (rectMax.y - rectMin.y) * ImGuiEx::M3::HALF
    );
    drawList->ChannelsMerge();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + m3Styles[Spacing::L]);
}

void FontPreviewPanel::DrawFontsTable(const std::vector<FontInfo> &fontInfos, const ImGuiEx::M3::M3Styles &m3Styles)
{
    using Spacing      = ImGuiEx::M3::Spacing;
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    m_interactState.interact = false;

    const auto &text = m3Styles.TitleText();
    ImGui::PushFont(nullptr, text.fontSize);
    {
        const auto itemSpacing = ImVec2(m3Styles[Spacing::L], m3Styles[Spacing::M]);

        ImGuiEx::StyleGuard styleGuard;
        styleGuard
            .Style_ItemSpacing(itemSpacing) // Selectable used
            .Style_SelectableTextAlign({0.f, 0.5f})
            .Style_ScrollbarSize(m3Styles[Spacing::XS])
            .Color_Header(m3Styles.Colors().at(SurfaceToken::surface))
            .Color_HeaderActive(m3Styles.Colors().Pressed(SurfaceToken::surface, ContentToken::onSurface))
            .Color_HeaderHovered(m3Styles.Colors().Hovered(SurfaceToken::surface, ContentToken::onSurface));

        ImGui::Spacing();
        auto &displayFontInfos = m_displayFontInfos.empty() ? fontInfos : m_displayFontInfos;
        // Inside the Table, ImGui does not respect the ItemSpacing settings,
        // and we need to simulate a Label with padding.
        ImGui::Indent(m3Styles[Spacing::L]);
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(displayFontInfos.size()));
        while (clipper.Step())
        {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                const auto &fontInfo = displayFontInfos[static_cast<size_t>(row)];
                ImGui::PushID(row);

                const bool selected = m_interactState.selectedIndex == fontInfo.GetIndex();
                auto       label    = std::format("{} {}", fontInfo.GetIndex() + 1, fontInfo.GetName());
                const auto clicked  = ImGui::Selectable(
                    fontInfo.GetName().c_str(),
                    selected,
                    ImGuiEx::SelectableFlags().SpanAllColumns(),
                    {0.f, text.lineHeight}
                );
                if (clicked && !selected)
                {
                    m_interactState.selectedIndex = fontInfo.GetIndex();
                    m_previewDebounceTimer.Poke();
                    m_state = State::DEBOUNCING;
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
                ImGui::PopID();
            }
        }
        ImGui::Unindent(m3Styles[Spacing::L]);
    }
    ImGui::PopFont();

    if (m_previewDebounceTimer.Check())
    {
        m_state             = State::EMPTY;
        const size_t fontId = static_cast<size_t>(m_interactState.selectedIndex);
        if (fontId >= fontInfos.size())
        {
            m_previewDebounceTimer.Reset();
        }
        else
        {
            auto      &fontInfo = fontInfos[fontId];
            const auto filePath = FontManager::GetFontFilePath(fontInfo);
            m_imFont.Cleanup();
            if (!filePath.empty())
            {
                PreviewFont(fontInfo.GetName(), filePath);
                m_state = State::PREVIEWING;
            }
            else
            {
                m_state = State::NOT_SUPPORTED_FONTS;
            }
        }
    }
}

void FontPreviewPanel::UpdateDisplayFontInfos(const std::vector<FontInfo> &sourceList)
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

} // namespace Ime
