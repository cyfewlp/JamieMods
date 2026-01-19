//
// Created by jamie on 2026/1/15.
//
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/LayoutHelper.h"
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

static std::string PREVIEW_TEXT = R"(!@#$%^&*()_+-=[]{}|;':",.<>?/
-- Unicode & Fallback --
Latín: áéíóú ñ  |  FullWidth: ＡＢＣ１２３
CJK: 繁體中文测试 / 简体中文测试 / 日本語 / 한국어
-- Emoji & Variation --
Icons: 🥰💀✌︎🌴🐢🐐🍄🍻👑📸😬👀🚨🏡
New: 🐦‍🔥 🍋‍🟩 🍄‍🟫 🙂‍↕️ 🙂‍↔️
-- Skyrim Immersion --
Dovah: Dovahkiin, naal ok zin los vahriin!
"I used to be an adventurer like you..."
)";

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
    m_baseFont.Cleanup();

    return true;
}

void FontBuilderView::Draw(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    if (!ImGui::CollapsingHeader(translation["$Font_Builder"])) return;
    DrawFontInfoTable(fontBuilder);

    DrawToolBar(fontBuilder, translation, settings);

    DrawHelpModal(translation);
    DrawWarningsModal(translation);

    if (auto state = m_fontPreviewPanel.Draw(fontBuilder, translation, settings); state.interact)
    {
        if (fontBuilder.AddFont(state.selectedIndex, m_fontPreviewPanel.GetImFont()))
        {
            m_fontPreviewPanel.Cleanup();
        }
    }
}

void FontBuilderView::DrawFontInfoTable(const FontBuilder &fontBuilder)
{
    auto listStyle = Material3Styles::LIST_4DENSITY;
    ImGui::PushFont(nullptr, listStyle.fontSize);
    ImGuiEx::StyleScope()
        // No effect: ImGui won't use cell padding when no vertical borders
        // .PushVar(ImGuiStyleVar_CellPadding, listStyle.padding)
        .PushVar(ImGuiStyleVar_ScrollbarSize, Material3Styles::CUSTOM_THICK_SCROLL_BAR_SIZE)
        .Draw([&fontBuilder, &listStyle] {
            ImGui::Indent(listStyle.padding.x);
            constexpr int MAX_DISPLAY_ROWS = 3;
            float         cellHeight       = listStyle.fontSize + listStyle.supportFontSize + listStyle.padding.y * 2.f;
            if (fontBuilder.IsBuilding() &&
                ImGui::BeginTable(
                    "BasicFontInfo",
                    2,
                    ImGuiEx::TableFlags().BordersInnerH().ScrollY().NoBordersInBody().SizingFixedFit(),
                    ImVec2(-FLT_MIN, cellHeight * MAX_DISPLAY_ROWS)
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
            ImGui::Unindent(listStyle.padding.x);
        });

    ImGui::PopFont();
}

/**
 * Use The @c ImDrawList::ChannelsXXX to render toolbar background after draw toolbar;
 */
void FontBuilderView::DrawToolBar(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    auto toolbar = Material3Styles::TOOL_BAR_STANDARD;

    // 1. apply the toolbar padding space to toolbar
    ImGui::SetCursorPos({ImGui::GetCursorPosX() + toolbar.padding.x, ImGui::GetCursorPosY() + toolbar.padding.y});

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    constexpr int bgChannel     = 0;
    constexpr int buttonChannel = 1;
    drawList->ChannelsSetCurrent(buttonChannel);
    ImGui::BeginGroup();
    DrawToolBarButtons(fontBuilder, translation, settings);
    ImGui::EndGroup();

    ImVec2 p_min = ImGui::GetItemRectMin();
    ImVec2 p_max = ImGui::GetItemRectMax();
    drawList->ChannelsSetCurrent(bgChannel);
    drawList->AddRectFilled(
        {p_min.x - toolbar.padding.x, p_min.y - toolbar.padding.y},
        {p_max.x + toolbar.padding.x, p_max.y + toolbar.padding.y},
        ImGui::GetColorU32(ImGuiCol_Button),
        toolbar.rounding
    );
    drawList->ChannelsMerge();

    // 2. apply the toolbar padding space to toolbar
    ImGui::SetCursorPos({ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + toolbar.padding.y});
}

void FontBuilderView::DrawToolBarButtons(FontBuilder &fontBuilder, const Translation &translation, Settings &settings)
{
    auto styleCount = LayoutHelper::PushButtonStyles(Material3Styles::XSMALL_ICON_BUTTON, true);

    ImGui::BeginDisabled(!fontBuilder.IsBuilding());
    if (ImGui::Button(ICON_MD_CHECK_DECAGRAM))
    {
        fontBuilder.ApplyFont(settings);
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_SetAsDefault"]);

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_RESTORE))
    {
        fontBuilder.Reset();
    }
    ImGui::SetItemTooltip("%s", translation["$Font_Builder_Reset"]);

    ImGui::SameLine();
    if (ImGui::Button(ICON_MD_EYE))
    {
        m_fontPreviewPanel.PreviewFont(fontBuilder.GetBaseFont());
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

auto FontPreviewPanel::Draw(FontBuilder &fontBuilder, const Translation &translation, const Settings &settings)
    -> InteractState
{
    constexpr auto MAX_ROWS         = 12;
    const float    TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    const ImVec2   FontViewerSize   = {200.0F, TEXT_BASE_HEIGHT * MAX_ROWS};

    m_interactState.interact = false;

    ImGui::PushID(1);
    ImGui::BeginDisabled(!m_imFont.IsCommittable());
    if (ImGui::Button(std::format("{} {}", ICON_MD_CONTENT_SAVE_MOVE, translation["$Add"]).c_str()))
    {
        if (m_interactState.selectedIndex >= 0)
        {
            m_interactState.interact = true;
        }
    }
    ImGui::SameLine();
    if (m_debounceTimer.IsWaiting())
    {
        ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextLink), ICON_MD_REFRESH);
    }
    else
    {
        ImGui::TextWrapped("%s %s", ICON_FA_FILE, m_imFont.GetFontPathOr(0).c_str());
    }
    ImGui::EndDisabled();
    ImGui::PopID();

    ImGui::BeginChild("#FontViewer", FontViewerSize, ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders);
    DrawSearchBox();

    DrawFontsTable(fontBuilder);
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("#FontPreviewer");
    {
        if (m_imFont)
        {
            ImGui::PushFont(m_imFont.UnsafeGetFont(), settings.state.fontSize);

            ImGui::InputTextMultiline(
                "##PreviewText",
                PREVIEW_TEXT.data(),
                PREVIEW_TEXT.capacity() + 1,
                ImVec2(-FLT_MIN, TEXT_BASE_HEIGHT * MAX_ROWS),
                ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_WordWrap
            );
            ImGui::PopFont();
        }
    }
    ImGui::EndChild();

    return m_interactState;
}

void FontPreviewPanel::DrawSearchBox()
{
    constexpr auto box      = Material3Styles::SEARCH_STANDARD;
    ImDrawList    *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit(2);

    drawList->ChannelsSetCurrent(1);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + box.padding.y);
    ImGui::BeginGroup();
    ImGui::SetNextItemWidth(-box.padding.x - box.fontSize);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGuiEx::StyleScope()
        .PushVarX(ImGuiStyleVar_ItemSpacing, 0.f)
        .PushVar(ImGuiStyleVar_FramePadding, box.padding)
        .PushColor(ImGuiCol_FrameBg, {0, 0, 0, 0})
        .Draw([this] {
            ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
            if (ImGui::InputTextWithHint(
                    "##Filter",
                    "search by name",
                    m_filter.InputBuf,
                    IM_COUNTOF(m_filter.InputBuf),
                    ImGuiInputTextFlags_EscapeClearsAll
                ))
            {
                m_filter.Build();
            }
            ImGui::PopItemFlag();
        });
    ImGui::SameLine();
    ImGui::Text(ICON_OCT_SEARCH);
    ImGui::EndGroup();

    const auto rectMin = ImGui::GetItemRectMin();
    const auto rectMax = ImGui::GetItemRectMax();
    drawList->ChannelsSetCurrent(0);
    drawList->AddRectFilled(
        rectMin, {rectMax.x + box.padding.x, rectMax.y}, ImGui::GetColorU32(ImGuiCol_FrameBg), box.rounding
    );
    drawList->ChannelsMerge();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + box.padding.y);
}

void FontPreviewPanel::DrawFontsTable(FontBuilder &fontBuilder)
{
    auto listStyle = Material3Styles::LIST_4DENSITY;
    ImGui::PushFont(nullptr, listStyle.fontSize);
    ImGuiEx::StyleScope()
        // No effect: ImGui won't use cell padding when no vertical borders
        // .PushVar(ImGuiStyleVar_CellPadding, listStyle.padding)
        .PushVar(ImGuiStyleVar_ScrollbarSize, Material3Styles::CUSTOM_THICK_SCROLL_BAR_SIZE)
        .Draw([&fontBuilder, &listStyle, this] {
            if (ImGui::BeginTable(
                    "#InstalledFonts", 2, ImGuiEx::TableFlags().SizingFixedFit().BordersInnerH().NoBordersInBody()
                ))
            {
                const auto &fontInfoList = fontBuilder.GetFontManager().GetFontInfoList();
                for (int idx = 0; static_cast<size_t>(idx) < fontInfoList.size(); idx++)
                {
                    const auto &fontInfo = fontInfoList[idx];
                    if (!m_filter.PassFilter(fontInfo.GetName().c_str()))
                    {
                        continue;
                    }
                    ImGui::TableNextRow();
                    ImGui::PushID(idx);

                    ImGui::TableNextColumn();
                    ImGui::SameLine(0, listStyle.padding.x * 2);
                    ImGui::Text("%d", idx + 1);

                    ImGui::TableNextColumn();
                    const bool selected = m_interactState.selectedIndex == idx;
                    if (ImGui::Selectable(
                            fontInfo.GetName().c_str(), selected, ImGuiEx::SelectableFlags().SpanAllColumns()
                        ) &&
                        !selected)
                    {
                        m_interactState.selectedIndex = idx;
                        m_debounceTimer.Poke();
                    }
                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();

                if (m_debounceTimer.Check())
                {
                    size_t fontId = static_cast<size_t>(m_interactState.selectedIndex);
                    if (fontId >= fontInfoList.size())
                    {
                        m_debounceTimer.Reset();
                    }
                    else
                    {
                        auto      &fontInfo = fontInfoList[fontId];
                        const auto filePath = FontManager::GetFontFilePath(fontInfo);
                        if (!filePath.empty())
                        {
                            PreviewFont(fontInfo.GetName(), filePath);
                        }
                    }
                }
            }
        });
    ImGui::PopFont();
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