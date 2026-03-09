//
// Created by jamie on 2026/1/15.
//
#define IMGUI_DEFINE_MATH_OPERATORS

#include "i18n/Translator.h"
#include "i18n/TranslatorHolder.h"
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

void UI::FontBuilderPanel::Draw(FontBuilder &fontBuilder, Settings &settings)
{
    auto &m3Styles = ImGuiEx::M3::Context::GetM3Styles();
    m_PreviewPanel.Draw(fontBuilder, m3Styles);
    ImGui::SameLine(0, M3Spec::Layout::ExtraLarge::Margin);
    {
        // The Font Builder child window.
        // \todo should support resize-x.
        const auto styleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_WindowBg>(m3Styles.Colors()[M3Spec::ColorRole::surfaceContainer]);

        // right-align, fixed width. The left two child windows(submitted in preview panel)
        // should auto resize along with the window resizing, and this child window should keep a fixed width
        // and align to the right side of the panel.
        const auto width = m3Styles.GetPixels(M3Spec::Layout::ExtraLarge::SideSheetsMaxWidth);
        if (ImGui::BeginChild("FontBuilderFontInfo", {width, 0}, ImGuiEx::ChildFlags()))
        {
            DrawToolBar(fontBuilder, settings);

            if (m_PreviewPanel.IsPreviewing())
            {
                DrawFontInfoTable(fontBuilder);
            }
        }
        ImGui::EndChild();
    }
}

void UI::FontBuilderPanel::DrawFontInfoTable(const FontBuilder &fontBuilder)
{
    using Spacing   = ImGuiEx::M3::Spacing;
    using ColorRole = M3Spec::ColorRole;

    if (!fontBuilder.IsBuilding())
    {
        return;
    }
    auto      &m3Styles   = ImGuiEx::M3::Context::GetM3Styles();
    const auto styleGuard = ImGuiEx::StyleGuard()
                                .Color<ImGuiCol_Border>(m3Styles.Colors().at(ColorRole::outlineVariant))
                                .Color<ImGuiCol_TableRowBg>(m3Styles.Colors().at(ColorRole::surface))
                                .Color<ImGuiCol_TableRowBgAlt>(m3Styles.Colors().at(ColorRole::surface))
                                .Style<ImGuiStyleVar_ScrollbarSize>(m3Styles[Spacing::XS]);
    ImGui::Indent(m3Styles[Spacing::L]);
    if (ImGui::BeginTable("BasicFontInfo", 2, ImGuiEx::TableFlags().BordersInnerH().ScrollY().NoBordersInBody().SizingFixedFit()))
    {
        auto &names = fontBuilder.GetBaseFont().GetFontNames();
        auto &paths = fontBuilder.GetBaseFont().GetFontPathList();
        if (names.size() == paths.size())
        {
            const auto labelLargeScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();
            for (size_t idx = 0U; idx < names.size(); idx++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGuiEx::M3::TextUnformatted(std::format("{}", idx + 1U));

                ImGui::TableNextColumn();

                ImGui::Text("%s", names.at(idx).c_str());
                {
                    const auto supportTextStyleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_Text>(m3Styles.Colors().at(ColorRole::onSurfaceVariant));
                    const auto labelSmallScope       = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelSmall>();
                    ImGui::Text("%s", paths.at(idx).c_str());
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::Indent(m3Styles[Spacing::L]);
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

    if (toolBar.Icon(ICON_ROTATE_CCW))
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
        ImGui::OpenPopup(Translate("Settings.FontBuilder.HelpTitle"));
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
