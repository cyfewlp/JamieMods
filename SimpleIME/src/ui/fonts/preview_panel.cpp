//
// Created by jamie on 2026/2/8.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui/fonts/preview_panel.h"

#include "common/i18n/Translator.h"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"
#include "common/imgui/imguiex_enum_wrap.h"
#include "common/imgui/imguiex_m3.h"
#include "common/imgui/m3_specs.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/fonts/FontManager.h"
#include "ui/fonts/FontPreviewPanel.h"
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
    if (UI::SearchBox(m_textFilter, m3Styles))
    {
        m_searchDebounceTimer.Poke();
    }

    if (UI::FontsTable(m_interactState.selectedIndex, m_displayFontInfos, m3Styles))
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

auto UI::SearchBox(ImGuiTextFilter &filter, const ImGuiEx::M3::M3Styles &m3Styles) -> bool
{
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;
    using SearchSpec   = ImGuiEx::M3::Spec::Search;

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    drawList->ChannelsSetCurrent(1);
    ImGui::PushFont(nullptr, m3Styles.TitleText().fontSize);

    ImRect bb(ImGui::GetCursorScreenPos(), {});
    ImGui::SetCursorScreenPos({bb.Min.x + SearchSpec::paddingX, bb.Min.y + SearchSpec::paddingY});
    ImGui::BeginGroup();
    bool edited = false;
    {
        ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
        ImGuiEx::M3::DrawIcon(ICON_OCT_SEARCH, m3Styles, ContentToken::onSurfaceVariant);
        ImGui::SameLine(0, SearchSpec::gap);

        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style_FramePadding({0, 0})
            .Color_Text(m3Styles.Colors().at(ContentToken::onSurface))
            .Color_FrameBg({0, 0, 0, 0});
        edited = ImGui::InputTextWithHint(
            "##Filter",
            "search by name",
            filter.InputBuf,
            IM_COUNTOF(filter.InputBuf),
            ImGuiInputTextFlags_EscapeClearsAll
        );
        ImGui::PopItemFlag();
    }
    ImGui::EndGroup();
    ImGui::PopFont();
    drawList->ChannelsSetCurrent(0);

    if (ImGui::IsItemVisible())
    {
        const auto rectMax = ImGui::GetItemRectMax();
        bb.Max             = {rectMax.x + SearchSpec::paddingX, rectMax.y + SearchSpec::paddingY};
        drawList->AddRectFilled(
            bb.Min,
            bb.Max,
            ImGui::ColorConvertFloat4ToU32(m3Styles.Colors().at(SurfaceToken::surfaceContainerHigh)),
            m3Styles.GetUnit(SearchSpec::rounding)
        );
        ImGui::ItemAdd(bb, 0);
    }
    drawList->ChannelsMerge();

    return edited;
}

auto UI::FontsTable(
    FontInfo::Index &selectedIndex, const std::vector<FontInfo> &fontInfos, const ImGuiEx::M3::M3Styles &m3Styles
) -> bool
{
    using Spacing      = ImGuiEx::M3::Spacing;
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    const auto &text = m3Styles.TitleText();
    ImGui::PushFont(nullptr, text.fontSize);
    const auto paddingX    = m3Styles.GetUnit(ImGuiEx::M3::Spec::List::paddingX);
    const auto paddingY    = m3Styles.GetUnit(ImGuiEx::M3::Spec::List::paddingY);
    const auto itemSpacing = ImVec2(paddingX, paddingY);

    ImGuiEx::StyleGuard styleGuard;
    styleGuard
        .Style_ItemSpacing(itemSpacing) // Selectable used
        .Style_SelectableTextAlign({ImGuiEx::ALIGN_LEFT, ImGuiEx::ALIGN_CENTER})
        .Style_ScrollbarSize(m3Styles[Spacing::XS])
        .Color_Header(m3Styles.Colors().at(SurfaceToken::surface))
        .Color_HeaderActive(m3Styles.Colors().Pressed(SurfaceToken::surface, ContentToken::onSurface))
        .Color_HeaderHovered(m3Styles.Colors().Hovered(SurfaceToken::surface, ContentToken::onSurface));

    ImGui::Spacing();

    // If use the SpanAllColumns flags, Selectable does not respect the ItemSpacing settings,
    // and we need to simulate a Label with padding.
    ImGui::Indent(paddingX);
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
            const bool clicked  = ImGui::Selectable(
                fontInfo.GetName().c_str(),
                selected,
                ImGuiEx::SelectableFlags().SpanAllColumns(),
                {0.f, text.lineHeight}
            );
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
    ImGui::Unindent(paddingX);
    ImGui::PopFont();

    return selectedNew;
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
