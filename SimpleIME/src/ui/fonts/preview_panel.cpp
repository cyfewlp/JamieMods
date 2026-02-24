//
// Created by jamie on 2026/2/8.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui/fonts/preview_panel.h"

#include "i18n/Translator.h"
#include "i18n/TranslatorHolder.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/facade/base.h"
#include "ui/fonts/FontManager.h"
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

auto SearchBox(ImGuiTextFilter &filter, const ImGuiEx::M3::M3Styles &m3Styles) -> bool
{
    using ColorRole  = M3Spec::ColorRole;
    using SearchSpec = ImGuiEx::M3::Spec::Search;

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    drawList->ChannelsSetCurrent(1);
    const auto fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::BodyLarge>();

    ImRect bb(ImGui::GetCursorScreenPos(), {});
    ImGui::SetCursorScreenPos({bb.Min.x + m3Styles.GetPixels(SearchSpec::paddingX), bb.Min.y + m3Styles.GetPixels(SearchSpec::paddingY)});
    ImGui::BeginGroup();
    bool edited = false;
    {
        ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
        ImGuiEx::M3::SmallIcon(ICON_OCT_SEARCH, m3Styles);
        ImGui::SameLine(0, m3Styles.GetPixels(SearchSpec::gap));

        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style<ImGuiStyleVar_FramePadding>({0, m3Styles.GetPixels(SearchSpec::paddingY)})
            .Color<ImGuiCol_Text>(m3Styles.Colors().at(ColorRole::onSurface))
            .Color<ImGuiCol_FrameBg>({0, 0, 0, 0});
        edited =
            ImGui::InputTextWithHint("##Filter", "search by name", filter.InputBuf, IM_COUNTOF(filter.InputBuf), ImGuiInputTextFlags_EscapeClearsAll);
        ImGui::PopItemFlag();
    }
    ImGui::EndGroup();
    drawList->ChannelsSetCurrent(0);

    if (ImGui::IsItemVisible())
    {
        const auto rectMax = ImGui::GetItemRectMax();
        bb.Max             = {rectMax.x + SearchSpec::paddingX, rectMax.y + SearchSpec::paddingY};
        drawList->AddRectFilled(
            bb.Min,
            bb.Max,
            ImGui::ColorConvertFloat4ToU32(m3Styles.Colors().at(ColorRole::surfaceContainerHigh)),
            m3Styles.GetPixels(SearchSpec::rounding)
        );
        ImGui::ItemSize(bb);
    }
    drawList->ChannelsMerge();

    return edited;
}

/**
 * @param selectedIndex current selected FontInfo index. will be set if select a new.
 * @return is select a new row.
 */
auto FontsTable(FontInfo::Index &selectedIndex, const std::vector<FontInfo> &fontInfos, const ImGuiEx::M3::M3Styles &m3Styles) -> bool
{
    using Spacing   = ImGuiEx::M3::Spacing;
    using ColorRole = M3Spec::ColorRole;

    const auto labelLargeScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();
    const auto paddingX        = m3Styles.GetPixels(ImGuiEx::M3::Spec::List::paddingX);
    const auto paddingY        = m3Styles.GetPixels(ImGuiEx::M3::Spec::List::paddingY);
    const auto itemSpacing     = ImVec2(paddingX, paddingY);

    ImGuiEx::StyleGuard styleGuard;
    styleGuard
        .Style<ImGuiStyleVar_ItemSpacing>(itemSpacing) // Selectable used
        .Style<ImGuiStyleVar_SelectableTextAlign>({ImGuiEx::ALIGN_LEFT, ImGuiEx::ALIGN_CENTER})
        .Style<ImGuiStyleVar_ScrollbarSize>(m3Styles[Spacing::XS])
        .Color<ImGuiCol_Header>(m3Styles.Colors().at(ColorRole::surface))
        .Color<ImGuiCol_HeaderActive>(m3Styles.Colors().Pressed(ColorRole::surface, ColorRole::onSurface))
        .Color<ImGuiCol_HeaderHovered>(m3Styles.Colors().Hovered(ColorRole::surface, ColorRole::onSurface));

    ImGui::Spacing();

    // If we use the SpanAllColumns flags, Selectable does not respect the ItemSpacing settings,
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
                fontInfo.GetName().c_str(), selected, ImGuiEx::SelectableFlags().SpanAllColumns(), {0.F, m3Styles.GetLastText().currText.lineHeight}
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

    return selectedNew;
}

void DrawStatusBar(const StatusBar &statusBar, const ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGuiEx::M3::SmallIcon(statusBar.icon, m3Styles);
    ImGui::SameLine();
    const auto fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();
    ImGui::PushTextWrapPos(0.0F);
    ImGuiEx::M3::TextUnformatted(statusBar.text, m3Styles, M3Spec::ColorRole::onSecondaryContainer);
    ImGui::PopTextWrapPos();

    ImGui::Separator();
    ImGui::Dummy({0, m3Styles[ImGuiEx::M3::Spacing::L]});
}

/**
 * contains a "apply" button to ensure if add font to @c FontBuilder
 * @return true if click the "apply" button, false otherwise.
 */
auto PreviewPanel(const ImGuiEx::M3::M3Styles &m3Styles) -> bool
{
    for (const auto &text : PREVIEW_TEXT)
    {
        ImGui::TextUnformatted(text);
    }
    const auto &avail   = ImGui::GetContentRegionAvail();
    const auto  marginY = m3Styles[ImGuiEx::M3::Spacing::L];
    ImGui::Dummy({0, marginY});
    constexpr auto fabSizeUnit = ImGuiEx::M3::Spec::FAB<ImGuiEx::M3::Spec::SizeTips::MEDIUM>::size;
    const auto     fabSize     = m3Styles.GetPixels(fabSizeUnit);
    if (const auto space = fabSize + marginY; avail.y > space)
    {
        ImGui::Dummy({0, avail.y - space});
    }
    const auto indent     = (avail.x - fabSize) * 0.5F;
    const auto safeIndent = ImMax(indent, 0.0F);
    ImGui::Indent(safeIndent);

    const bool clicked = ImGuiEx::M3::FAB<ImGuiEx::M3::Spec::SizeTips::MEDIUM>(ICON_MD_TRANSFER_RIGHT, m3Styles);
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Add"), m3Styles);

    ImGui::Unindent(safeIndent);
    return clicked;
}

} // namespace

void FontPreviewPanel::DrawFontsView(const std::vector<FontInfo> &fontInfos, const ImGuiEx::M3::M3Styles &m3Styles)
{
    if (SearchBox(m_textFilter, m3Styles))
    {
        m_searchDebounceTimer.Poke();
    }

    const auto &fontInfos1 = m_displayFontInfos.empty() ? fontInfos : m_displayFontInfos;
    if (FontsTable(m_interactState.selectedIndex, fontInfos1, m3Styles))
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
            const auto  filePath = FontManager::GetFontFilePath(fontInfo);
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

void FontPreviewPanel::Draw(FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles)
{
    {
        const auto width = m3Styles.GetPixels(ImGuiEx::M3::Spec::List::width);

        const auto styleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_WindowBg>(m3Styles.Colors()[M3Spec::ColorRole::surfaceContainer]);
        if (ImGui::BeginChild("FontsView", {width, 0}, ImGuiEx::ChildFlags().Borders()))
        {
            DrawFontsView(fontBuilder.GetFontManager().GetFontInfoList(), m3Styles);
        }
        ImGui::EndChild();
    }

    if (m_state == State::NOT_SUPPORTED_FONTS)
    {
        return;
    }

    StatusBar statusBar;
    statusBar.icon = ICON_FA_FILE;
    switch (m_state)
    {
        case State::DEBOUNCING:
            statusBar.icon = ICON_MD_REFRESH;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.Debouncing");
            break;
        case State::PREVIEW_BUILDER_FONT:
            statusBar.icon = ICON_MD_EYE;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.BuilderFont");
            break;
        case State::NOT_SELECTED_FONT: {
            statusBar.icon = ICON_FA_CIRCLE_INFO;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.NotSelectedFont");
            break;
        }
        case State::NOT_SUPPORTED_FONTS: {
            statusBar.icon = ICON_FA_CIRCLE_EXCLAMATION;
            statusBar.text = Translate("Settings.FontBuilder.PreviewPanel.NotSupportedFont");
            break;
        }
        case State::PREVIEWING:
            statusBar.text = m_imFont.GetFontPathOr(0);
            break;
        default:;
    }

    const auto styleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_WindowBg>(m3Styles.Colors()[M3Spec::ColorRole::surfaceContainerLow]);
    if (ImGui::BeginChild("PreviewPanel", {}, ImGuiEx::ChildFlags().AutoResizeX()))
    {
        DrawStatusBar(statusBar, m3Styles);
        ImGui::BeginDisabled(!m_imFont.IsCommittable());
        ImGui::PushFont(m_imFont.UnsafeGetFont(), 0);
        auto clicked = PreviewPanel(m3Styles);
        ImGui::PopFont();
        ImGui::EndDisabled();
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

} // namespace Ime::UI
