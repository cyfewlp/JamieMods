//
// Created by jamie on 2026/1/15.
//
#include "common/utils.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontBuilderView.h"

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

void FontBuilder::UpdatePreviewFont(const FontInfo &fontInfo)
{
    const auto filePath = FontManager::GetFontFilePath(fontInfo);
    if (!filePath.empty())
    {
        m_previewFont.SetProperty(fontInfo.name, filePath);
    }
}

void FontBuilder::BuildPreviewFont()
{
    if (!m_previewFont.IsWantUpdate())
    {
        return;
    }
    ReleasePreviewFont();
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    const auto &path            = m_previewFont.GetFilePath();
    m_previewFont.SetImFont(ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), 0, &config));
}

auto FontBuilder::SetBaseFont() -> void
{
    if (m_baseFont != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(m_baseFont);
        m_fontNames.clear();
        m_fontPathList.clear();
    }
    m_baseFont = m_previewFont.GetImFont();

    // Transfer memory ownership to ImGui
    for (const auto &config : m_baseFont->Sources)
    {
        config->FontDataOwnedByAtlas = true;
    }

    m_fontNames.push_back(std::move(m_previewFont.GetFullName()));
    m_fontPathList.push_back(std::move(m_previewFont.GetFilePath()));
    m_previewFont.Reset();
}

void FontBuilder::MergeFont()
{
    if (MergeFont(m_previewFont.GetImFont(), m_previewFont.GetFullName(), m_previewFont.GetFilePath()))
    {
        m_previewFont.Reset();
    }
}

bool FontBuilder::MergeFont(ImFont *imFont, std::string_view fullName, std::string_view fontPath)
{
    if (imFont == nullptr)
    {
        return false;
    }
    const auto *fontConfig = imFont->Sources[0];
    assert(!fontConfig->FontDataOwnedByAtlas && "Keep FontDataOwnedByAtlas to avoid font data copy!");

    ImFontConfig config;
    ImStrncpy(config.Name, fontConfig->Name, IM_COUNTOF(config.Name));
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH  = true;
    config.MergeMode   = true;
    config.DstFont     = m_baseFont;
    if (!fontConfig->FontDataOwnedByAtlas)
    {
        // Dont set FontDataOwnedByAtlas to false.
        config.FontData     = fontConfig->FontData;
        config.FontDataSize = fontConfig->FontDataSize;
    }
    else // should not reachable
    {
        config.FontData     = ImGui::MemAlloc(fontConfig->FontDataSize);
        config.FontDataSize = fontConfig->FontDataSize;
        std::memcpy(config.FontData, fontConfig->FontData, config.FontDataSize);
    }

    auto &io = ImGui::GetIO();
    io.Fonts->AddFont(&config);
    io.Fonts->RemoveFont(imFont);

    m_fontNames.emplace_back(fullName);
    m_fontPathList.emplace_back(fontPath);
    return true;
}

void FontBuilder::Preview()
{
    if (auto *imFont = m_previewFont.GetImFont(); /**/
        m_previewFont.IsFontOwner() && imFont != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(imFont);
    }
    m_previewFont.SetPreviewImFont(m_baseFont);
}

void FontBuilder::SetAsDefault(Settings &settings)
{
    auto &io = ImGui::GetIO();
    if (io.FontDefault != m_baseFont)
    {
        io.Fonts->RemoveFont(io.FontDefault);
        io.FontDefault = m_baseFont;
    }

    ImFontConfig config;
    config.MergeMode   = true;
    config.OversampleH = 1;
    config.OversampleV = 1;
    config.PixelSnapH  = true;
    config.DstFont     = m_baseFont;

    // Note: Automatic detection of existing icon glyphs is skipped due
    // to implementation complexity.
    const auto iconFile = CommonUtils::GetInterfaceFile(Settings::ICON_FILE);
    io.Fonts->AddFontFromFileTTF(iconFile.c_str(), 0.0F, &config);

    settings.resources.fontPathList = std::move(m_fontPathList);

    m_baseFont = nullptr;
    m_fontNames.clear();
}

void FontBuilder::Reset()
{
    if (m_baseFont != nullptr)
    {
        ImGui::GetIO().Fonts->RemoveFont(m_baseFont);
    }
    m_baseFont = nullptr;
    m_fontNames.clear();
    m_fontPathList.clear();
}

void FontBuilder::ReleasePreviewFont()
{
    if (m_previewFont.IsFontOwner() && m_previewFont.GetImFont() != nullptr)
    {
        for (const auto &config : m_previewFont.GetImFont()->Sources)
        {
            if (!config->FontDataOwnedByAtlas)
            {
                IM_FREE(config->FontData);
            }
        }
        ImGui::GetIO().Fonts->RemoveFont(m_previewFont.GetImFont());
    }
    m_previewFont.SetPreviewImFont(nullptr);
}

void FontBuilderView::Draw(FontBuilder &fontBuilder, Settings &settings)
{
    ImGui::SeparatorText(m_translation["$Font_Builder"]);

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
        for (const auto &name : fontBuilder.GetFontNames())
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
        if (ImGui::Button(m_translation["$Font_Builder_SetAsDefault"]))
        {
            fontBuilder.SetAsDefault(settings);
        }

        ImGui::SameLine();
        if (ImGui::Button(m_translation["$Font_Builder_Reset"]))
        {
            fontBuilder.Reset();
        }

        ImGui::SameLine();
        if (ImGui::Button(m_translation["$Font_Builder_Preview"]))
        {
            fontBuilder.Preview();
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
    DrawHelpModal();
    DrawWarningsModal();

    if (DrawFontViewer(fontBuilder, settings))
    {
        if (!fontBuilder.IsBuilding())
        {
            fontBuilder.SetBaseFont();
        }
        else // merge to base font
        {
            fontBuilder.MergeFont();
        }
    }
}

bool FontBuilderView::DrawFontViewer(FontBuilder &fontBuilder, const Settings &settings)
{
    static int selectedIndex = -1;

    constexpr auto MAX_ROWS         = 12;
    const float    TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    const ImVec2   FontViewerSize   = {200.0F, TEXT_BASE_HEIGHT * MAX_ROWS};

    bool        applied     = false;
    const auto &previewFont = fontBuilder.GetPreviewFont();

    ImGui::PushID(1);
    ImGui::BeginDisabled(!previewFont.IsCommittable());
    if (ImGui::Button(std::format("{} {}", ICON_MD_CONTENT_SAVE_MOVE, m_translation["$Add"]).c_str()))
    {
        if (selectedIndex >= 0)
        {
            applied       = true;
            selectedIndex = -1;
        }
    }
    ImGui::SameLine();
    ImGui::TextWrapped("%s %s", ICON_FA_FILE, previewFont.GetFilePath().c_str());
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
                selectedIndex = idx;
                fontBuilder.UpdatePreviewFont(fontInfo);
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
        if (previewFont.GetImFont() != nullptr)
        {
            ImGui::PushFont(previewFont.GetImFont(), settings.fontSizeTemp);

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

void FontBuilderView::DrawHelpModal() const
{
    if (ImGui::BeginPopupModal(TITLE_HELP, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", m_translation["$Font_Builder_Help1"]);
        ImGui::Text("%s", m_translation["$Font_Builder_Help2"]);
        ImGui::NewLine();
        ImGui::Text("%s", m_translation["$Font_Builder_Help3"]);

        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5F - ImGui::GetFontSize());
        if (ImGui::Button(ICON_FA_OK_SIGN))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void FontBuilderView::DrawWarningsModal() const
{
    if (ImGui::BeginPopupModal(TITLE_WARNINGS, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s %s", ICON_FA_WARNING, m_translation["$Font_Builder_Warning1"]);
        ImGui::NewLine();
        ImGui::Text("%s %s", ICON_FA_WARNING, m_translation["$Font_Builder_Warning2"]);

        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5F - ImGui::GetFontSize());
        if (ImGui::Button(ICON_FA_OK_SIGN))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
}
}