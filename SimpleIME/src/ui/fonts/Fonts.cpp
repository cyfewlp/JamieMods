//
// Created by jamie on 2026/1/15.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Layouts.h"
#include "common/imgui/Material3Styles.h"
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
    : font(imFont)
{
    m_fontNames.emplace_back(fontName);
    m_fontPathList.emplace_back(fontPath);
    owner = a_owner && font != nullptr;
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

    ImFontConfig config;
    config.MergeMode   = true;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH  = true;
    config.DstFont     = imFont;

    // Note: Automatic detection of existing icon glyphs is skipped due
    // to implementation complexity.
    const auto iconFile = CommonUtils::GetInterfaceFile(Settings::ICON_FILE);
    io.Fonts->AddFontFromFileTTF(iconFile.c_str(), 0.0F, &config);

    settings.resources.fontPathList = std::move(m_baseFont.GetFontPathList());
    Reset();
    return true;
}

void FontBuilderView::Draw(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    if (!ImGui::CollapsingHeader(translation["$Font_Builder"])) return;

    constexpr struct
    {
        float height  = Material3Styles::GRID_UNIT * -16; // sticky bottom border
        float width   = Material3Styles::GRID_UNIT * 70;
        float padding = Material3Styles::GRID_UNIT * 8;
    } BoxStyle; // box: child/group

    if (ImGui::BeginChild("FontsTable", {BoxStyle.width, BoxStyle.height}, ImGuiEx::ChildFlags().Borders().ResizeX()))
    {
        m_PreviewPanel.DrawFontsView(fontBuilder.GetFontManager().GetFontInfoList());
    }
    ImGui::EndChild();

    ImGui::SameLine(0, BoxStyle.padding);
    if (ImGui::BeginChild(
            "FontsPreviewer", {BoxStyle.width, BoxStyle.height}, ImGuiEx::ChildFlags().Borders().ResizeX()
        ))
    {
        m_PreviewPanel.DrawFontsPreviewView(translation);
    }
    ImGui::EndChild();

    ImGui::SameLine(0, Material3Styles::GRID_UNIT);
    constexpr auto ADD_BUTTON_OFFSET_Y = Material3Styles::STANDARD_FONT_SIZE * 10;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ADD_BUTTON_OFFSET_Y);
    DrawAddFontButton(fontBuilder, translation);
    ImGui::SameLine(0, Material3Styles::GRID_UNIT);

    ImGui::BeginGroup();
    if (ImGuiEx::BeginRightAlign("#ToolBar"))
    {
        DrawToolBar(fontBuilder, translation, settings);
        ImGuiEx::EndRightAlign();
    }
    if (m_PreviewPanel.IsWaitingPreview())
    {
        if (ImGui::BeginChild("FontBuilderFontInfo", {-FLT_MIN, BoxStyle.height}))
        {
            DrawFontInfoTable(fontBuilder);
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();
}

void FontBuilderView::DrawAddFontButton(FontBuilder &fontBuilder, const Translation &translation)
{
    ImGui::BeginDisabled(!m_PreviewPanel.IsWaitingCommit());
    ImGui::PushFont(nullptr, Material3Styles::MEDIUM_ICON_BUTTON.fontSize);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Material3Styles::MEDIUM_ICON_BUTTON.padding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, Material3Styles::MEDIUM_ICON_BUTTON.rounding);
    if (ImGui::Button(ICON_MD_TRANSFER_RIGHT))
    {
        if (fontBuilder.AddFont(m_PreviewPanel.GetInteractState().selectedIndex, m_PreviewPanel.GetImFont()))
        {
            m_PreviewPanel.Cleanup();
        }
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_Add"]);
    ImGui::PopStyleVar(2);
    ImGui::PopFont();
    ImGui::EndDisabled();
}

void FontBuilderView::DrawFontInfoTable(const FontBuilder &fontBuilder)
{
    if (!fontBuilder.IsBuilding())
    {
        return;
    }
    auto listStyle = Material3Styles::LIST_4DENSITY;
    ImGui::PushFont(nullptr, listStyle.fontSize);
    ImGuiEx::StyleScope()
        // No effect: ImGui won't use cell padding when no vertical borders
        // .PushVar(ImGuiStyleVar_CellPadding, listStyle.padding)
        .PushVar(ImGuiStyleVar_ScrollbarSize, Material3Styles::CUSTOM_THICK_SCROLL_BAR_SIZE)
        .Draw([&fontBuilder, &listStyle] {
            if (ImGui::BeginTable(
                    "BasicFontInfo",
                    2,
                    ImGuiEx::TableFlags().BordersInnerH().ScrollY().NoBordersInBody().SizingFixedFit()
                ))
            {
                auto &names = fontBuilder.GetBaseFont().GetFontNames();
                auto &paths = fontBuilder.GetBaseFont().GetFontPathList();
                if (names.size() == paths.size())
                {
                    for (size_t idx = 0; idx < names.size(); idx++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::SameLine(0, listStyle.padding.x * 2);
                        ImGui::Text("%llu", idx + 1);

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", names[idx].c_str());
                        ImGui::PushFont(nullptr, listStyle.supportFontSize);
                        ImGui::Text("%s", paths[idx].c_str());
                        ImGui::PopFont();
                    }
                }
                ImGui::EndTable();
            }
        });
    ImGui::PopFont();
}

/**
 * Use The @c ImDrawList::ChannelsXXX to render toolbar background after draw toolbar;
 */
void FontBuilderView::DrawToolBar(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    auto toolbar = Material3Styles::TOOL_BAR_STANDARD;

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    constexpr int bgChannel     = 0;
    constexpr int buttonChannel = 1;
    drawList->ChannelsSetCurrent(buttonChannel);
    ImVec2 toolbarLT = ImGui::GetCursorScreenPos();
    ImGui::NewLine();

    ImGui::SetCursorScreenPos(toolbarLT + toolbar.padding);

    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0}); // avoid button color mix bg color
    DrawToolBarButtons(fontBuilder, translation, settings);
    ImGui::PopStyleColor();
    ImGui::EndGroup();

    drawList->ChannelsSetCurrent(bgChannel);
    drawList->AddRectFilled(
        toolbarLT, ImGui::GetItemRectMax() + toolbar.padding, ImGui::GetColorU32(ImGuiCol_Button), toolbar.rounding
    );
    drawList->ChannelsMerge();

    DrawHelpModal(translation);
    DrawWarningsModal(translation);
}

void FontBuilderView::DrawToolBarButtons(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    auto styleCount = ImGuiEx::PushButtonStyles(Material3Styles::SMALL_ICON_BUTTON, true);

    ImGui::PushFont(nullptr, Material3Styles::SMALL_ICON_BUTTON.fontSize);
    ImGui::BeginDisabled(!fontBuilder.IsBuilding());
    if (ImGui::Button(ICON_FA_WRENCH))
    {
        fontBuilder.ApplyFont(settings);
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_SetAsDefault"]);

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_RESTORE))
    {
        if (fontBuilder.GetBaseFont() == m_PreviewPanel.GetImFont())
        {
            m_PreviewPanel.Cleanup();
        }
        fontBuilder.Reset();
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_Reset"]);

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_EYE))
    {
        m_PreviewPanel.PreviewFont(fontBuilder.GetBaseFont());
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_Preview"]);
    ImGui::EndDisabled();

    ImGui::SameLine();
    auto centerPopup = [](std::string_view name) {
        ImGui::OpenPopup(name.data());
        constexpr auto CENTER_PIVOT = ImVec2(0.5f, 0.5f);

        const auto viewportSize = ImGui::GetMainViewport()->Size;
        ImGui::SetNextWindowSize({viewportSize.x * 0.75f, 0.f}, ImGuiCond_Always);
        ImGui::SetNextWindowPos({viewportSize.x * 0.5f, viewportSize.y * 0.5f}, ImGuiCond_Always, CENTER_PIVOT);
    };
    if (ImGui::Button(ICON_MD_ALERT_CIRCLE_OUTLINE))
    {
        centerPopup(TITLE_WARNINGS);
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_Warning"]);
    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_HELP_CIRCLE_OUTLINE))
    {
        centerPopup(TITLE_HELP);
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_Help"]);
    ImGui::PopFont();
    ImGui::PopStyleVar(styleCount);
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
        ImGui::Text("%s", translation["$Font_Builder_Warning2"]);
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
    if (m_previewDebounceTimer.IsWaiting())
    {
        ImGui::Text("%s", ICON_MD_REFRESH);
    }
    else
    {
        auto tips = std::format("{} {}", ICON_FA_CIRCLE_INFO, translation["$Font_Builder_Preview_EmptyFontTips"]);
        ImGui::TextWrapped("%s %s", ICON_FA_FILE, m_imFont.GetFontPathOr(0, tips).c_str());
    }

    ImGui::Separator();
    ImGui::Dummy({0, Material3Styles::GRID_UNIT * 4});

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

void FontPreviewPanel::DrawSearchBox(const std::vector<FontInfo> &fontInfos)
{
    constexpr auto box      = Material3Styles::SEARCH_STANDARD;
    ImDrawList    *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    drawList->ChannelsSetCurrent(1);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + box.padding.y);
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(-box.padding.x - box.fontSize);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    auto drawAction = [this] {
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
    };
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    ImGuiEx::StyleScope()
        .PushVarX(ImGuiStyleVar_ItemSpacing, 0.f)
        .PushVar(ImGuiStyleVar_FramePadding, box.padding)
        .PushColor(ImGuiCol_FrameBg, {0, 0, 0, 0})
        .Draw(drawAction);
    ImGui::PopItemFlag();
    ImGui::SameLine();
    ImGui::Text(ICON_OCT_SEARCH);
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
        rectMin, {rectMax.x + box.padding.x, rectMax.y}, ImGui::GetColorU32(ImGuiCol_FrameBg), box.rounding
    );
    drawList->ChannelsMerge();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + box.padding.y);
}

void FontPreviewPanel::DrawFontsTable(const std::vector<FontInfo> &fontInfos)
{
    m_interactState.interact = false;
    auto &displayFontInfos   = m_displayFontInfos.empty() ? fontInfos : m_displayFontInfos;
    auto  drawAction         = [this, &displayFontInfos] {
        if (ImGui::BeginTable(
                "#InstalledFonts",
                4,
                ImGuiEx::TableFlags().ScrollY().SizingFixedFit().BordersInnerH().NoBordersInBody(),
                {-FLT_MIN, -FLT_MIN}
            ))
        {
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(displayFontInfos.size()));
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    const auto &fontInfo = displayFontInfos[static_cast<size_t>(row)];
                    ImGui::TableNextRow();
                    ImGui::PushID(row);

                    ImGui::TableNextColumn();
                    ImGui::SameLine(0, Material3Styles::LIST_4DENSITY.padding.x * 2);
                    ImGui::Text("%d", fontInfo.GetIndex() + 1);

                    ImGui::TableNextColumn();
                    const bool selected = m_interactState.selectedIndex == fontInfo.GetIndex();
                    if (ImGui::Selectable(
                            fontInfo.GetName().c_str(), selected, ImGuiEx::SelectableFlags().SpanAllColumns()
                        ) &&
                        !selected)
                    {
                        m_interactState.selectedIndex = fontInfo.GetIndex();
                        m_previewDebounceTimer.Poke();
                    }
                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
    };
    ImGui::PushFont(nullptr, Material3Styles::LIST_4DENSITY.fontSize);
    ImGuiEx::StyleScope()
        // No effect: ImGui won't use cell padding when no vertical borders
        // .PushVar(ImGuiStyleVar_CellPadding, Material3Styles::XSMALL_ICON_BUTTON.padding)
        .PushVar(ImGuiStyleVar_FramePadding, ImVec2{})
        .PushVar(ImGuiStyleVar_ScrollbarSize, Material3Styles::CUSTOM_THICK_SCROLL_BAR_SIZE)
        .Draw(drawAction);

    if (m_previewDebounceTimer.Check())
    {
        size_t fontId = static_cast<size_t>(m_interactState.selectedIndex);
        if (fontId >= fontInfos.size())
        {
            m_previewDebounceTimer.Reset();
        }
        else
        {
            auto      &fontInfo = fontInfos[fontId];
            const auto filePath = FontManager::GetFontFilePath(fontInfo);
            if (!filePath.empty())
            {
                PreviewFont(fontInfo.GetName(), filePath);
            }
        }
    }
    ImGui::PopFont();
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
}

}
}