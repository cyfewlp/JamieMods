//
// Created by jamie on 2026/1/15.
//
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
{
    font = imFont;
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
    if (owner && font != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(font);
    }
}

constexpr auto FontBuilder::IsBuilding() const -> bool
{
    return m_baseFont.IsCommittable();
}

auto FontBuilder::SetBaseFont(ImFontWrap &imFont) -> void
{
    m_baseFont = std::move(imFont);
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
    ImGui::SeparatorText(translation["$Font_Builder"]);

    // draw chosen font information
    if (fontBuilder.IsBuilding() &&
        ImGui::BeginTable(
            "BasicFontInfo",
            2,
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3)
        ))
    {
        int idx = 0;
        for (const auto &name : fontBuilder.GetBaseFont().GetFontNames())
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d", idx + 1);
            ImGui::TableNextColumn();
            ImGui::Text("%s", name.c_str());
            idx++;
        }
        ImGui::EndTable();
        ImGui::BeginDisabled(!fontBuilder.IsBuilding());
        if (ImGui::Button(translation["$Font_Builder_SetAsDefault"]))
        {
            fontBuilder.ApplyFont(settings);
        }

        ImGui::SameLine();
        if (ImGui::Button(translation["$Font_Builder_Reset"]))
        {
            fontBuilder.Reset();
        }

        ImGui::SameLine();
        if (ImGui::Button(translation["$Font_Builder_Preview"]))
        {
            m_fontPreviewPanel.PreviewFont(fontBuilder.GetBaseFont());
        }
        ImGui::EndDisabled();

        ImGui::Separator();
    }

    if (ImGui::Button(ICON_MD_HELP_BOX))
    {
        ImGui::OpenPopup(TITLE_HELP);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_WARNING))
    {
        ImGui::OpenPopup(TITLE_WARNINGS);
    }
    DrawHelpModal(translation);
    DrawWarningsModal(translation);

    if (m_fontPreviewPanel.Draw(fontBuilder, translation, settings))
    {
        if (!fontBuilder.IsBuilding())
        {
            fontBuilder.SetBaseFont(m_fontPreviewPanel.GetImFont());
        }
        else // merge to base font
        {
            fontBuilder.MergeFont(m_fontPreviewPanel.GetImFont());
        }
        m_fontPreviewPanel.Cleanup();
    }
}

void FontBuilderView::DrawHelpModal(const Translation &translation)
{
    if (ImGui::BeginPopupModal(TITLE_HELP, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", translation["$Font_Builder_Help1"]);
        ImGui::Text("%s", translation["$Font_Builder_Help2"]);
        ImGui::NewLine();
        ImGui::Text("%s", translation["$Font_Builder_Help3"]);

        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5F - ImGui::GetFontSize());
        if (ImGui::Button(ICON_FA_OK_SIGN))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void FontBuilderView::DrawWarningsModal(const Translation &translation)
{
    if (ImGui::BeginPopupModal(TITLE_WARNINGS, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s %s", ICON_FA_WARNING, translation["$Font_Builder_Warning1"]);
        ImGui::NewLine();
        ImGui::Text("%s %s", ICON_FA_WARNING, translation["$Font_Builder_Warning2"]);

        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5F - ImGui::GetFontSize());
        if (ImGui::Button(ICON_FA_OK_SIGN))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool FontPreviewPanel::Draw(FontBuilder &fontBuilder, const Translation &translation, const Settings &settings)
{
    static int selectedIndex = -1;

    constexpr auto MAX_ROWS         = 12;
    const float    TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    const ImVec2   FontViewerSize   = {200.0F, TEXT_BASE_HEIGHT * MAX_ROWS};

    bool applied = false;

    ImGui::PushID(1);
    ImGui::BeginDisabled(!m_imFont.IsCommittable());
    if (ImGui::Button(std::format("{} {}", ICON_MD_CONTENT_SAVE_MOVE, translation["$Add"]).c_str()))
    {
        if (selectedIndex >= 0)
        {
            applied       = true;
            selectedIndex = -1;
        }
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s %s", ICON_FA_FILE, m_imFont.GetFontPathOr(0).c_str());
    ImGui::EndDisabled();
    ImGui::PopID();

    ImGui::BeginChild("#FontViewer", FontViewerSize, ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders);
    constexpr auto flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg |
                           ImGuiTableFlags_NoHostExtendX;

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F, ImGuiInputFlags_Tooltip);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    if (ImGui::InputTextWithHint(
            "##Filter",
            "search font by name",
            m_filter.InputBuf,
            IM_COUNTOF(m_filter.InputBuf),
            ImGuiInputTextFlags_EscapeClearsAll
        ))
    {
        m_filter.Build();
    }
    ImGui::PopItemFlag();

    if (ImGui::BeginTable("#InstalledFonts", 2, flags, ImVec2(0.0F, TEXT_BASE_HEIGHT * MAX_ROWS)))
    {
        int idx = 0;
        for (const auto &fontInfo : fontBuilder.GetFontManager().GetFontInfoList())
        {
            if (!m_filter.PassFilter(fontInfo.name.c_str()))
            {
                continue;
            }
            ImGui::TableNextRow();
            ImGui::PushID(idx);

            ImGui::TableNextColumn();
            ImGui::Text("%d", idx + 1);

            ImGui::TableNextColumn();
            const bool selected = selectedIndex == idx;
            if (ImGui::Selectable(fontInfo.name.c_str(), selected) && !selected)
            {
                selectedIndex       = idx;
                const auto filePath = FontManager::GetFontFilePath(fontInfo);
                if (!filePath.empty())
                {
                    PreviewFont(fontInfo.name, filePath);
                }
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();

            idx++;
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("#FontPreviewer");
    {
        if (m_imFont)
        {
            ImGui::PushFont(m_imFont.UnsafeGetFont(), settings.fontSizeTemp);

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

    return applied;
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