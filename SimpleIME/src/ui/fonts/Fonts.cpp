//
// Created by jamie on 2026/1/15.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "common/i18n/Translator.h"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"
#include "common/imgui/imgui_m3_ex.h"
#include "i18n/TranslatorHolder.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontBuilderPanel.h"
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

ImFontWrap::ImFontWrap(ImFont *imFont, std::string_view fontName, std::string_view fontPath, bool a_owner)
    : font(imFont), owner(a_owner && font != nullptr)
{
    m_fontNames.emplace_back(fontName);
    m_fontPathList.emplace_back(fontPath);
}

ImFontWrap &ImFontWrap::operator=(const ImFontWrap &other)
{
    if (this == &other) return *this;
    RemoveFontIfOwned();

    font           = other.font;
    owner          = false;
    m_fontNames    = other.m_fontNames;
    m_fontPathList = other.m_fontPathList;
    return *this;
}

ImFontWrap &ImFontWrap::operator=(ImFontWrap &&other) noexcept
{
    if (this == &other) return *this;
    RemoveFontIfOwned();

    font           = other.font;
    owner          = other.owner;
    m_fontNames    = std::move(other.m_fontNames);
    m_fontPathList = std::move(other.m_fontPathList);

    other.font  = nullptr;
    other.owner = false;
    return *this;
}

ImFontWrap::~ImFontWrap()
{
    Cleanup();
}

bool ImFontWrap::IsCommittable() const
{
    return owner && font != nullptr;
}

bool ImFontWrap::IsCommittableSingleFont() const
{
    return owner && font != nullptr && font->Sources.size() == 1;
}

void ImFontWrap::Cleanup()
{
    RemoveFontIfOwned();
    font  = nullptr;
    owner = false;
    m_fontNames.clear();
    m_fontPathList.clear();
}

void ImFontWrap::RemoveFontIfOwned() const
{
    if (owner && font != nullptr && ImGui::GetCurrentContext() != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(font);
    }
}

constexpr auto FontBuilder::IsBuilding() const -> bool
{
    return m_baseFont.IsCommittable();
}

bool FontBuilder::AddFont(int fontId, ImFontWrap &imFont)
{
    bool result = true;

    if (std::ranges::contains(m_usedFontIds, fontId)) // already used
    {
        return false;
    }

    if (!IsBuilding())
    {
        m_baseFont = std::move(imFont);
    }
    else // merge to base font
    {
        result = MergeFont(imFont);
    }
    if (result)
    {
        m_usedFontIds.push_back(fontId);
    }
    return result;
}

bool FontBuilder::MergeFont(ImFontWrap &imFontWrap)
{
    if (!imFontWrap.IsCommittableSingleFont())
    {
        return false;
    }

    auto *imFont                     = imFontWrap.TakeFont();
    auto *fontConfig                 = imFont->Sources[0];
    fontConfig->FontDataOwnedByAtlas = false; // avoid copy font data

    ImFontConfig config;
    ImStrncpy(config.Name, fontConfig->Name, IM_COUNTOF(config.Name));
    config.OversampleH  = 1;
    config.OversampleV  = 1;
    config.PixelSnapH   = true;
    config.MergeMode    = true;
    config.DstFont      = m_baseFont.UnsafeGetFont();
    config.FontData     = fontConfig->FontData;
    config.FontDataSize = fontConfig->FontDataSize;

    ImGui::GetIO().Fonts->AddFont(&config);
    ImGui::GetIO().Fonts->RemoveFont(imFont);

    m_baseFont.AddFontInfo(imFontWrap.GetFontNameOr(0), imFontWrap.GetFontPathOr(0));
    return true;
}

bool FontBuilder::ApplyFont(Settings &settings)
{
    if (!m_baseFont.IsCommittable())
    {
        return false;
    }

    auto &io = ImGui::GetIO();
    if (auto *imFont = m_baseFont.TakeFont(); io.FontDefault != imFont)
    {
        io.Fonts->RemoveFont(io.FontDefault);
        io.FontDefault = imFont;
    }
    settings.resources.fontPathList = std::move(m_baseFont.GetFontPathList());
    Reset();
    return true;
}

void FontBuilderPanel::Draw(FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles)
{
    using Spacing       = ImGuiEx::M3::Spacing;
    using ComponentSize = ImGuiEx::M3::ComponentSize;
    using SurfaceToken  = ImGuiEx::M3::SurfaceToken;

    auto                minWidth = m3Styles.GetSize(ComponentSize::LIST_WIDTH);
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color_ChildBg(m3Styles.Colors().at(SurfaceToken::surfaceContainerHighest));
    {
        ImGui::SetNextWindowSizeConstraints(
            {minWidth, 0.f}, {std::max(minWidth, ImGui::GetContentRegionAvail().x - minWidth), FLT_MAX}
        );
        if (ImGui::BeginChild(
                "FontsTable", {minWidth, -FLT_MIN}, ImGuiEx::ChildFlags().AlwaysUseWindowPadding().ResizeX()
            ))
        {
            ImGui::Dummy({0, m3Styles[Spacing::L]});
            ImGui::Indent(m3Styles[Spacing::M]);
            m_PreviewPanel.DrawFontsView(fontBuilder.GetFontManager().GetFontInfoList(), m3Styles);
            ImGui::Unindent(m3Styles[Spacing::M]);
        }
        ImGui::EndChild();
    }
    ImGui::SameLine(0, 0);
    {
        ImGuiEx::StyleGuard styleGuard1;
        styleGuard1.Color_ChildBg(m3Styles.Colors().at(SurfaceToken::surfaceContainerLowest))
            .Style_WindowPadding({m3Styles[Spacing::L], m3Styles[Spacing::L]});

        ImGui::SetNextWindowSizeConstraints(
            {m3Styles[Spacing::XL], 0.f},
            {std::max(m3Styles[Spacing::XL], ImGui::GetContentRegionAvail().x - minWidth), FLT_MAX}
        );
        if (ImGui::BeginChild(
                "FontsPreviewer",
                {-m3Styles.GetSize(ComponentSize::NAV_RAIL_WIDTH) * 2.f, -FLT_MIN},
                ImGuiEx::ChildFlags().Borders().ResizeX().AlwaysUseWindowPadding()
            ))
        {
            m_PreviewPanel.DrawFontsPreviewView(m3Styles);

            const auto width  = m3Styles.LabelText().fontSize + m3Styles[Spacing::Double_L];
            const auto height = width;
            const auto avail  = ImGui::GetContentRegionAvail();
            ImGui::Dummy({0.f, avail.y - height - m3Styles[Spacing::XL]});
            const auto indent = (avail.x - width) * .5f;
            ImGui::Indent(indent);
            DrawAddFontButton(fontBuilder, m3Styles);
            ImGui::Unindent(indent);
        }
        ImGui::EndChild();
    }
    ImGui::SameLine(0, 0);
    {
        if (ImGui::BeginChild("FontBuilderFontInfo", {-FLT_MIN, -FLT_MIN}, ImGuiEx::ChildFlags()))
        {
            DrawToolBar(fontBuilder, settings, m3Styles);

            if (m_PreviewPanel.IsWaitingPreview())
            {
                DrawFontInfoTable(fontBuilder, m3Styles);
            }
        }
        ImGui::EndChild();
    }
}

void FontBuilderPanel::DrawAddFontButton(FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGui::BeginDisabled(!m_PreviewPanel.IsWaitingCommit());
    {
        if (ImGuiEx::M3::DrawIconButton(
                ICON_MD_TRANSFER_RIGHT,
                m3Styles,
                ImGuiEx::M3::SurfaceToken::primary,
                ImGuiEx::M3::ContentToken::onPrimary,
                ImGuiEx::M3::SizeTips::MEDIUM
            ))
        {
            if (fontBuilder.AddFont(m_PreviewPanel.GetInteractState().selectedIndex, m_PreviewPanel.GetImFont()))
            {
                m_PreviewPanel.Cleanup();
            }
        }
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Add"), m3Styles);
    ImGui::EndDisabled();
}

void FontBuilderPanel::DrawFontInfoTable(const FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles) const
{
    using Spacing      = ImGuiEx::M3::Spacing;
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    if (!fontBuilder.IsBuilding())
    {
        return;
    }
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color_Border(m3Styles.Colors().at(SurfaceToken::outlineVariant))
        .Color_TableRowBg(m3Styles.Colors().at(SurfaceToken::surface))
        .Color_TableRowBgAlt(m3Styles.Colors().at(SurfaceToken::surface))
        .Style_ScrollbarSize(m3Styles[Spacing::XS]);
    ImGui::Indent(m3Styles[Spacing::L]);
    if (ImGui::BeginTable(
            "BasicFontInfo", 2, ImGuiEx::TableFlags().BordersInnerH().ScrollY().NoBordersInBody().SizingFixedFit()
        ))
    {
        auto &names = fontBuilder.GetBaseFont().GetFontNames();
        auto &paths = fontBuilder.GetBaseFont().GetFontPathList();
        if (names.size() == paths.size())
        {
            ImGui::PushFont(nullptr, m3Styles.TitleText().fontSize);
            const auto col1LineHeight = m3Styles.TitleText().fontSize + m3Styles.SmallLabelText().fontSize;
            for (size_t idx = 0; idx < names.size(); idx++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGuiEx::M3::LineTextUnformatted(std::format("{}", idx + 1), col1LineHeight);

                ImGui::TableNextColumn();

                ImGui::Text("%s", names.at(idx).c_str());
                {
                    ImGuiEx::StyleGuard styleGuard1;
                    styleGuard1.Color_Text(m3Styles.Colors().at(ContentToken::onSurfaceVariant));
                    ImGui::PushFont(nullptr, m3Styles.SmallLabelText().fontSize);
                    ImGui::Text("%s", paths.at(idx).c_str());
                    ImGui::PopFont();
                }
            }
            ImGui::PopFont();
        }
        ImGui::EndTable();
    }
    ImGui::Indent(m3Styles[Spacing::L]);
}

void FontBuilderPanel::DrawToolBar(FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles)
{
    if (ImGuiEx::M3::BeginDockedToolbar(
            m3Styles.GetSize(ImGuiEx::M3::ComponentSize::ICON_BUTTON),
            5,
            ImGuiEx::M3::SurfaceToken::surfaceContainer,
            m3Styles
        ))
    {
        DrawToolBarButtons(fontBuilder, settings, m3Styles);
        ImGuiEx::M3::EndDockedToolbar();
    }

    DrawHelpModal();
    DrawWarningsModal();
}

void FontBuilderPanel::DrawToolBarButtons(
    FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles
)
{
    ImGui::BeginDisabled(!fontBuilder.IsBuilding());
    auto Button = [&m3Styles](const std::string_view &icon) -> bool {
        return ImGuiEx::M3::DrawIconButton(
            icon,
            m3Styles,
            ImGuiEx::M3::SurfaceToken::surfaceContainer,
            ImGuiEx::M3::ContentToken::onSurfaceVariant,
            ImGuiEx::M3::SizeTips::XSMALL
        );
    };

    if (ImGuiEx::M3::DrawIconButton(
            ICON_FA_WRENCH,
            m3Styles,
            ImGuiEx::M3::SurfaceToken::primary,
            ImGuiEx::M3::ContentToken::onPrimary,
            ImGuiEx::M3::SizeTips::XSMALL
        ))
    {
        fontBuilder.ApplyFont(settings);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.SetAsDefault"), m3Styles);

    ImGui::SameLine();
    if (Button(ICON_MD_RESTORE))
    {
        if (fontBuilder.GetBaseFont() == m_PreviewPanel.GetImFont())
        {
            m_PreviewPanel.Cleanup();
        }
        fontBuilder.Reset();
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Reset"), m3Styles);

    ImGui::SameLine();
    if (Button(ICON_MD_EYE))
    {
        m_PreviewPanel.PreviewFont(fontBuilder.GetBaseFont());
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Preview"), m3Styles);
    ImGui::EndDisabled();

    ImGui::SameLine();
    auto centerPopup = [](std::string_view name) {
        ImGui::OpenPopup(name.data());
        constexpr auto CENTER_PIVOT = ImVec2(0.5f, 0.5f);

        const auto viewportSize = ImGui::GetMainViewport()->Size;
        ImGui::SetNextWindowSize({viewportSize.x * 0.75f, 0.f}, ImGuiCond_Always);
        ImGui::SetNextWindowPos({viewportSize.x * 0.5f, viewportSize.y * 0.5f}, ImGuiCond_Always, CENTER_PIVOT);
    };
    if (ImGuiEx::M3::DrawIconButton(
            ICON_MD_ALERT_CIRCLE_OUTLINE,
            m3Styles,
            ImGuiEx::M3::SurfaceToken::tertiaryContainer,
            ImGuiEx::M3::ContentToken::onTertiaryContainer,
            ImGuiEx::M3::SizeTips::XSMALL
        ))
    {
        centerPopup(TITLE_WARNING);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Warning"), m3Styles);
    ImGui::SameLine();
    if (ImGuiEx::M3::DrawIconButton(
            ICON_MD_HELP_CIRCLE_OUTLINE,
            m3Styles,
            ImGuiEx::M3::SurfaceToken::secondaryContainer,
            ImGuiEx::M3::ContentToken::onSecondaryContainer,
            ImGuiEx::M3::SizeTips::XSMALL
        ))
    {
        centerPopup(TITLE_HELP);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Help"), m3Styles);
}

void FontBuilderPanel::DrawHelpModal()
{
    bool open = true;
    if (ImGui::BeginPopupModal(
            Translate("Settings.FontBuilder.HelpTitle"),
            &open,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar
        ))
    {
        ImGui::Text("%s", Translate("Settings.FontBuilder.Help1"));
        ImGui::Text("%s", Translate("Settings.FontBuilder.Help2"));
        ImGui::Text("%s", Translate("Settings.FontBuilder.Help3"));
        ImGui::EndPopup();
    }
}

void FontBuilderPanel::DrawWarningsModal()
{
    bool open = true;
    if (ImGui::BeginPopupModal(
            Translate("Settings.FontBuilder.WarningTitle"),
            &open,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar
        ))
    {
        ImGui::Text("%s", Translate("Settings.FontBuilder.Warning1"));
        ImGui::EndPopup();
    }
}

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
            .Color_HeaderActive(
                m3Styles.Colors().at(SurfaceToken::surface).Pressed(m3Styles.Colors().at(ContentToken::onSurface))
            )
            .Color_HeaderHovered(
                m3Styles.Colors().at(SurfaceToken::surface).Hovered(m3Styles.Colors().at(ContentToken::onSurface))
            );

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
