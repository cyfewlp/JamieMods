//
// Created by jamie on 2026/1/15.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"
#include "common/imgui/imgui_m3_ex.h"
#include "common/utils.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontBuilderView.h"
#include "ui/fonts/FontPreviewPanel.h"
#include "ui/fonts/ImFontWrap.h"

namespace LIBC_NAMESPACE_DECL
{
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

    auto &io     = ImGui::GetIO();
    auto *imFont = m_baseFont.TakeFont();
    if (io.FontDefault != imFont)
    {
        io.Fonts->RemoveFont(io.FontDefault);
        io.FontDefault = imFont;
    }
    settings.resources.fontPathList = std::move(m_baseFont.GetFontPathList());
    Reset();
    return true;
}

void FontBuilderView::Draw(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    constexpr struct
    {
        float height  = -FLT_MIN;
        float width   = ImGuiEx::M3::GRID_UNIT * 70;
        float padding = ImGuiEx::M3::GRID_UNIT * 8;
    } BoxStyle; // box: child/group

    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Push(ImGuiEx::ColorHolder::ChildBg(m_styles.colors.SurfaceContainerHighest()));
        ImGui::SetNextWindowSizeConstraints(
            {BoxStyle.width, 0.f},
            {std::max(BoxStyle.width, ImGui::GetContentRegionAvail().x - BoxStyle.width), FLT_MAX}
        );
        if (ImGui::BeginChild(
                "FontsTable",
                {BoxStyle.width, BoxStyle.height},
                ImGuiEx::ChildFlags().AlwaysUseWindowPadding().ResizeX()
            ))
        {
            ImGui::Dummy({0, ImGuiEx::M3::CUSTOM_WINDOW_PADDING2.y});
            ImGui::Indent(ImGuiEx::M3::CUSTOM_WINDOW_PADDING2.x);
            m_PreviewPanel.DrawFontsView(fontBuilder.GetFontManager().GetFontInfoList());
            ImGui::Unindent(ImGuiEx::M3::CUSTOM_WINDOW_PADDING2.x);
        }
        ImGui::EndChild();
    }
    ImGui::SameLine(0, 0);
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Push(ImGuiEx::ColorHolder::ChildBg(m_styles.colors.SurfaceContainerLowest()))
            .Push(ImGuiEx::StyleHolder::WindowPadding(ImGuiEx::M3::CUSTOM_WINDOW_PADDING2));
        if (ImGui::BeginChild(
                "FontsPreviewer",
                {-BoxStyle.width, BoxStyle.height},
                ImGuiEx::ChildFlags().Borders().AlwaysUseWindowPadding()
            ))
        {
            m_PreviewPanel.DrawFontsPreviewView(translation);

            constexpr auto button = ImGuiEx::M3::MEDIUM_ICON_BUTTON;
            constexpr auto width  = button.fontSize + button.padding.x * 2.f;
            constexpr auto height = button.fontSize + button.padding.y * 2.f;
            const auto     avail  = ImGui::GetContentRegionAvail();
            ImGui::Dummy({0.f, avail.y - height - ImGuiEx::M3::CUSTOM_WINDOW_PADDING1.y});
            const auto indent = (avail.x - width) * .5f;
            ImGui::Indent(indent);
            DrawAddFontButton(fontBuilder, translation);
            ImGui::Unindent(indent);
        }
        ImGui::EndChild();
    }
    ImGui::SameLine(0, 0);
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Push(ImGuiEx::ColorHolder::ChildBg(m_styles.colors.SurfaceContainerHighest()));
        if (ImGui::BeginChild("FontBuilderFontInfo", {-FLT_MIN, BoxStyle.height}, ImGuiEx::ChildFlags()))
        {
            DrawToolBar(fontBuilder, translation, settings);

            if (m_PreviewPanel.IsWaitingPreview())
            {
                DrawFontInfoTable(fontBuilder);
            }
        }
        ImGui::EndChild();
    }
}

void FontBuilderView::DrawAddFontButton(FontBuilder &fontBuilder, const Translation &translation)
{
    ImGui::BeginDisabled(!m_PreviewPanel.IsWaitingCommit());
    {
        if (ImGuiEx::M3::DrawIconButton(
                ICON_MD_TRANSFER_RIGHT,
                m_styles.colors.Primary(),
                m_styles.colors.OnPrimary(),
                m_styles.iconFont,
                ImGuiEx::M3::FAB::STANDARD
            ))
        {
            if (fontBuilder.AddFont(m_PreviewPanel.GetInteractState().selectedIndex, m_PreviewPanel.GetImFont()))
            {
                m_PreviewPanel.Cleanup();
            }
        }
    }
    ImGuiEx::M3::SetItemToolTip(translation["$Font_Builder_Add"], m_styles.colors);
    ImGui::EndDisabled();
}

void FontBuilderView::DrawFontInfoTable(const FontBuilder &fontBuilder) const
{
    if (!fontBuilder.IsBuilding())
    {
        return;
    }
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Push(ImGuiEx::ColorHolder::Border(m_styles.colors.OutlineVariant()))
        .Push(ImGuiEx::ColorHolder::TableRowBg(m_styles.colors.Surface()))
        .Push(ImGuiEx::ColorHolder::TableRowBgAlt(m_styles.colors.Surface()))
        .Push(ImGuiEx::StyleHolder::ScrollbarSize(ImGuiEx::M3::CUSTOM_THICK_SCROLL_BAR_SIZE));
    constexpr auto list = ImGuiEx::M3::List::STANDARD;
    ImGui::Indent(list.padding.x);
    if (ImGui::BeginTable(
            "BasicFontInfo", 2, ImGuiEx::TableFlags().BordersInnerH().ScrollY().NoBordersInBody().SizingFixedFit()
        ))
    {
        auto &names = fontBuilder.GetBaseFont().GetFontNames();
        auto &paths = fontBuilder.GetBaseFont().GetFontPathList();
        if (names.size() == paths.size())
        {
            ImGui::PushFont(nullptr, list.text.fontSize);
            constexpr auto col1LineHeight = list.text.lineHeight + list.supportText.lineHeight;
            for (size_t idx = 0; idx < names.size(); idx++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGuiEx::M3::LineTextUnformatted(std::format("{}", idx + 1), col1LineHeight);

                ImGui::TableNextColumn();

                ImGui::Text("%s", names[idx].c_str());
                {
                    ImGuiEx::StyleGuard styleGuard1;
                    styleGuard1.Push(ImGuiEx::ColorHolder::Text(m_styles.colors.OnSurfaceVariant()));
                    ImGui::PushFont(nullptr, list.supportText.fontSize);
                    ImGui::Text("%s", paths[idx].c_str());
                    ImGui::PopFont();
                }
            }
            ImGui::PopFont();
        }
        ImGui::EndTable();
    }
    ImGui::Unindent(list.padding.x);
}

void FontBuilderView::DrawToolBar(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    if (ImGuiEx::M3::BeginDockedToolbar(ImGuiEx::M3::IconButton::XSMALL.size, 5, m_styles.colors.SurfaceContainer()))
    {
        DrawToolBarButtons(fontBuilder, translation, settings);
        ImGuiEx::M3::EndDockedToolbar();
    }
    ImGui::Dummy({0, ImGuiEx::M3::Toolbar::DOCKED.margin.y});

    DrawHelpModal(translation);
    DrawWarningsModal(translation);
}

void FontBuilderView::DrawToolBarButtons(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    ImGui::BeginDisabled(!fontBuilder.IsBuilding());
    auto Button = [this](const std::string_view &icon) -> bool {
        return ImGuiEx::M3::DrawIconButton(
            icon,
            m_styles.colors.SurfaceContainer(),
            m_styles.colors.OnSurfaceVariant(),
            m_styles.iconFont,
            ImGuiEx::M3::IconButton::XSMALL
        );
    };

    if (ImGuiEx::M3::DrawIconButton(
            ICON_FA_WRENCH,
            m_styles.colors.Primary(),
            m_styles.colors.OnPrimary(),
            m_styles.iconFont,
            ImGuiEx::M3::IconButton::XSMALL
        ))
    {
        fontBuilder.ApplyFont(settings);
    }
    ImGuiEx::M3::SetItemToolTip(translation["$Font_Builder_SetAsDefault"], m_styles.colors);

    ImGui::SameLine();
    if (Button(ICON_MD_RESTORE))
    {
        if (fontBuilder.GetBaseFont() == m_PreviewPanel.GetImFont())
        {
            m_PreviewPanel.Cleanup();
        }
        fontBuilder.Reset();
    }
    ImGuiEx::M3::SetItemToolTip(translation["$Font_Builder_Reset"], m_styles.colors);

    ImGui::SameLine();
    if (Button(ICON_MD_EYE))
    {
        m_PreviewPanel.PreviewFont(fontBuilder.GetBaseFont());
    }
    ImGuiEx::M3::SetItemToolTip(translation["$Font_Builder_Preview"], m_styles.colors);
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
            m_styles.colors.TertiaryContainer(),
            m_styles.colors.OnTertiaryContainer(),
            m_styles.iconFont,
            ImGuiEx::M3::IconButton::XSMALL
        ))
    {
        centerPopup(TITLE_WARNINGS);
    }
    ImGuiEx::M3::SetItemToolTip(translation["Font_Builder_Warning"], m_styles.colors);
    ImGui::SameLine();
    if (ImGuiEx::M3::DrawIconButton(
            ICON_MD_HELP_CIRCLE_OUTLINE,
            m_styles.colors.SecondaryContainer(),
            m_styles.colors.OnSecondaryContainer(),
            m_styles.iconFont,
            ImGuiEx::M3::IconButton::XSMALL
        ))
    {
        centerPopup(TITLE_HELP);
    }
    ImGuiEx::M3::SetItemToolTip(translation["$Font_Builder_Help"], m_styles.colors);
}

void FontBuilderView::DrawHelpModal(const Translation &translation)
{
    bool open = true;
    if (ImGui::BeginPopupModal(TITLE_HELP, &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Text("%s", translation["$Font_Builder_Help1"]);
        ImGui::Text("%s", translation["$Font_Builder_Help2"]);
        ImGui::Text("%s", translation["$Font_Builder_Help3"]);
        ImGui::EndPopup();
    }
}

void FontBuilderView::DrawWarningsModal(const Translation &translation)
{
    bool open = true;
    if (ImGui::BeginPopupModal(TITLE_WARNINGS, &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Text("%s", translation["$Font_Builder_Warning1"]);
        ImGui::EndPopup();
    }
}

void FontPreviewPanel::DrawFontsView(const std::vector<FontInfo> &fontInfos)
{
    DrawSearchBox(fontInfos);
    DrawFontsTable(fontInfos);
}

void FontPreviewPanel::DrawFontsPreviewView(const Translation &translation) const
{
    DrawStatusBar(translation);

    ImGui::Separator();
    ImGui::Dummy({0, ImGuiEx::M3::GRID_UNIT * 4});

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

void FontPreviewPanel::DrawStatusBar(const Translation &translation) const
{
    std::string_view icon = "";
    std::string_view msg  = "";
    switch (m_state)
    {
        case State::DEBOUNCING:
            icon = ICON_MD_REFRESH;
            msg  = translation["$Font_Builder_Preview_Debouncing"];
            break;
        case State::PREVIEW_BUILDER_FONT:
            icon = ICON_MD_EYE;
            msg  = translation["$Font_Builder_Preview_BuilderFont"];
            break;
        case State::NOT_SELECTED_FONT: {
            icon = ICON_FA_CIRCLE_INFO;
            msg  = translation["$Font_Builder_Preview_NotSelectedFont"];
            break;
        }
        case State::NOT_SUPPORTED_FONTS: {
            icon = ICON_FA_CIRCLE_EXCLAMATION;
            msg  = translation["$Font_Builder_Preview_NotSupportedFont"];
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
        styleGuard.Push(ImGuiEx::ColorHolder::Text(m_styles.colors.OnSecondaryContainer()));
        if (!icon.empty())
        {
            ImGui::Text("%s", icon.data());
            ImGui::SameLine(0, ImGuiEx::M3::XSMALL_ICON_BUTTON.spacing.x);
        }
        ImGui::TextWrapped("%s", msg.data());
        ImGui::SameLine();
        ImGui::Dummy({8., 8.f + ImGui::GetFontSize()});
    }
}

void FontPreviewPanel::DrawSearchBox(const std::vector<FontInfo> &fontInfos)
{
    constexpr auto box      = ImGuiEx::M3::SEARCH_STANDARD;
    ImDrawList    *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    drawList->ChannelsSetCurrent(1);
    ImGui::BeginGroup();
    {
        ImGui::SetNextItemWidth(-box.padding.x - box.fontSize - ImGuiEx::M3::CUSTOM_WINDOW_PADDING2.x);
        ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Push(ImGuiEx::StyleHolder::FramePadding(box.padding))
            .Push(ImGuiEx::ColorHolder::Text(m_styles.colors.OnSurfaceVariant()))
            .Push(ImGuiEx::ColorHolder::FrameBg({0, 0, 0, 0}));
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
        ImGui::SameLine(0, box.padding.x);
        ImGui::Text(ICON_OCT_SEARCH);
    }
    ImGui::EndGroup();

    if (m_searchDebounceTimer.Check())
    {
        m_textFilter.Build();
        UpdateDisplayFontInfos(fontInfos);
    }

    const auto rectMin = ImGui::GetItemRectMin();
    const auto rectMax = ImGui::GetItemRectMax();
    drawList->ChannelsSetCurrent(0);
    drawList->AddRectFilled(
        rectMin, {rectMax.x + box.padding.x, rectMax.y}, m_styles.colors.SurfaceContainerHigh(), box.rounding
    );
    drawList->ChannelsMerge();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + box.padding.y);
}

void FontPreviewPanel::DrawFontsTable(const std::vector<FontInfo> &fontInfos)
{
    m_interactState.interact = false;

    constexpr auto list = ImGuiEx::M3::List::STANDARD;
    ImGui::PushFont(nullptr, list.text.fontSize);
    {
        constexpr auto      itemSpacing = ImVec2(list.padding.x, list.padding.y * 2.f);
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Push(ImGuiEx::StyleHolder::ItemSpacing(itemSpacing))
            .Push(ImGuiEx::StyleHolder::SelectableTextAlign({0.f, 0.5f}))
            .Push(ImGuiEx::StyleHolder::ScrollbarSize(ImGuiEx::M3::CUSTOM_THICK_SCROLL_BAR_SIZE))
            .Push(ImGuiEx::ColorHolder::Text(m_styles.colors.OnSurface()))
            .Push(ImGuiEx::ColorHolder::Header(m_styles.colors.Surface()))
            .Push(
                ImGuiEx::ColorHolder::HeaderActive(
                    m_styles.colors.Surface().GetPressedState(m_styles.colors.OnSurface())
                )
            )
            .Push(
                ImGuiEx::ColorHolder::HeaderHovered(
                    m_styles.colors.Surface().GetHoveredState(m_styles.colors.OnSurface())
                )
            );

        ImGui::Spacing();
        auto &displayFontInfos = m_displayFontInfos.empty() ? fontInfos : m_displayFontInfos;
        // Inside the Table, ImGui does not respect the ItemSpacing settings,
        // and we need to simulate a Label with padding.
        ImGui::Indent(list.padding.x);
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
                auto       clicked  = ImGui::Selectable(
                    fontInfo.GetName().c_str(),
                    selected,
                    ImGuiEx::SelectableFlags().SpanAllColumns(),
                    {0.f, list.text.lineHeight}
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
        ImGui::Unindent(list.padding.x);
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

}
}