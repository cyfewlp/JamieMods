//
// Created by jamie on 2026/1/15.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "i18n/Translator.h"
#include "i18n/translator_manager.h"
#include "icons.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/spec/layout.h"
#include "ui/fonts/FontBuilder.h"
#include "ui/fonts/FontBuilderPanel.h"
#include "ui/fonts/ImFontWrap.h"
#include "ui/fonts/preview_panel.h"

namespace Ime
{
namespace
{
//! Merge the source font into the target font.
//! The source font will be removed after merging, and the target font will be updated with the merged font.
//! The source font should be committable and only contains a single font source, otherwise the merge will fail.
auto MergeFont(ImFontWrap &target, ImFontWrap &source) -> bool
{
    if (!source.IsCommittableSingleFont())
    {
        return false;
    }

    auto *imFont                     = source.TakeFont();
    auto *fontConfig                 = imFont->Sources[0];
    fontConfig->FontDataOwnedByAtlas = false; // avoid copy font data

    ImFontConfig config;
    ImStrncpy(config.Name, fontConfig->Name, IM_COUNTOF(config.Name));
    config.OversampleH  = 1;
    config.OversampleV  = 1;
    config.PixelSnapH   = true;
    config.MergeMode    = true;
    config.DstFont      = target.UnsafeGetFont();
    config.FontData     = fontConfig->FontData;
    config.FontDataSize = fontConfig->FontDataSize;

    ImGui::GetIO().Fonts->AddFont(&config);
    ImGui::GetIO().Fonts->RemoveFont(imFont);

    target.AddFontInfo(source.GetFontNameOr(0), source.GetFontPathOr(0));
    return true;
}
} // namespace

ImFontWrap::ImFontWrap(ImFont *imFont, std::string_view fontName, std::string_view fontPath, bool a_owner)
    : font(imFont), owner(a_owner && font != nullptr)
{
    m_fontNames.emplace_back(fontName);
    m_fontPathList.emplace_back(fontPath);
}

auto ImFontWrap::operator=(const ImFontWrap &other) -> ImFontWrap &
{
    if (this == &other) return *this;
    RemoveFontIfOwned();

    font           = other.font;
    owner          = false;
    m_fontNames    = other.m_fontNames;
    m_fontPathList = other.m_fontPathList;
    return *this;
}

auto ImFontWrap::operator=(ImFontWrap &&other) noexcept -> ImFontWrap &
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

auto ImFontWrap::IsCommittable() const -> bool
{
    return owner && font != nullptr;
}

auto ImFontWrap::IsCommittableSingleFont() const -> bool
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

//! Add a font to the builder.
//! The font will be invalid after adding.
auto FontBuilder::AddFont(int fontId, ImFontWrap &imFont) -> bool
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
        result = MergeFont(m_baseFont, imFont);
    }
    if (result)
    {
        m_usedFontIds.push_back(fontId);
    }
    return result;
}

auto FontBuilder::ApplyFont(Settings &settings) -> bool
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

void UI::FontBuilderPanel::Draw(FontBuilder &fontBuilder, Settings &settings)
{
    auto &m3Styles = ImGuiEx::M3::Context::GetM3Styles();

    const auto styleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_ChildBg>(m3Styles.Colors()[M3Spec::ColorRole::surface]);
    m_PreviewPanel.Draw(fontBuilder);

    const auto margin = m3Styles.GetPixels(M3Spec::Layout::ExtraLarge::Margin);
    ImGui::SameLine(0, margin);
    ImGui::BeginGroup();
    {
        m_PreviewPanel.DrawPreviewPanel(fontBuilder, m3Styles);
        if (ImGui::BeginChild("FontBuilderFontInfo", {-margin, 0}, ImGuiEx::ChildFlags()))
        {
            DrawToolBar(fontBuilder, settings);
            DrawFontInfoTable(fontBuilder);
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();
}

void UI::FontBuilderPanel::DrawFontInfoTable(const FontBuilder &fontBuilder)
{
    if (!fontBuilder.IsBuilding())
    {
        return;
    }
    for (const auto &fontName : fontBuilder.GetBaseFont().GetFontNames())
    {
        ImGuiEx::M3::MenuItem(fontName, false);
    }
}

void UI::FontBuilderPanel::DrawToolBar(FontBuilder &fontBuilder, Settings &settings)
{
    if (const auto toolBar = ImGuiEx::M3::DockedToolBar("FontBuilderToolBar", 5))
    {
        DrawToolBarButtons(toolBar, fontBuilder, settings);

        DrawHelpModal();
        DrawWarningsModal();
    }
}

void UI::FontBuilderPanel::DrawToolBarButtons(const ImGuiEx::M3::DockedToolbarScope &toolBar, FontBuilder &fontBuilder, Settings &settings)
{
    ImGui::BeginDisabled(!fontBuilder.IsBuilding());
    if (toolBar.Icon(ICON_WRENCH))
    {
        fontBuilder.ApplyFont(settings);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.SetAsDefault"));

    if (toolBar.Icon(ICON_ROTATE_CCW)) // reset font builder
    {
        if (fontBuilder.GetBaseFont() == m_PreviewPanel.GetImFont())
        {
            m_PreviewPanel.Cleanup();
        }
        fontBuilder.Reset();
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Reset"));

    if (toolBar.Icon(ICON_EYE))
    {
        m_PreviewPanel.PreviewFont(fontBuilder.GetBaseFont());
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Preview"));
    ImGui::EndDisabled();

    if (toolBar.Icon(ICON_CIRCLE_ALERT))
    {
        ImGui::OpenPopup(TITLE_WARNING);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Warning"));

    if (toolBar.Icon(ICON_CIRCLE_QUESTION_MARK))
    {
        ImGui::OpenPopup(Translate("Settings.FontBuilder.HelpTitle").data());
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Help"));
}

void UI::FontBuilderPanel::DrawHelpModal()
{
    if (auto dialog = ImGuiEx::M3::DialogModal(Translate("Settings.FontBuilder.HelpTitle")); dialog)
    {
        dialog.SupportingText(Translate("Settings.FontBuilder.Help1"), true);
        dialog.SupportingText(Translate("Settings.FontBuilder.Help2"), true);
        dialog.ActionButton(Translate("Settings.Apply"));
    }
}

void UI::FontBuilderPanel::DrawWarningsModal()
{
    if (auto dialog = ImGuiEx::M3::DialogModal(Translate("Settings.FontBuilder.WarningTitle")); dialog)
    {
        dialog.SupportingText(Translate("Settings.FontBuilder.Warning1"), true);
        dialog.ActionButton(Translate("Settings.Apply"));
    }
}

} // namespace Ime
