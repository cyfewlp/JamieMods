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
#include "imguiex/m3/facade/button.h"
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

void UI::FontBuilderPanel::Draw(FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles)
{
    m_PreviewPanel.Draw(fontBuilder, m3Styles);
    ImGui::SameLine(0, 0);
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Color<ImGuiCol_WindowBg>(m3Styles.Colors()[ImGuiEx::M3::SurfaceToken::surfaceContainerHighest]);

        const auto width = m3Styles.GetPixels(ImGuiEx::M3::Spec::List::width);
        if (ImGui::BeginChild("FontBuilderFontInfo", {-width, -FLT_MIN}, ImGuiEx::ChildFlags()))
        {
            DrawToolBar(fontBuilder, settings, m3Styles);

            if (m_PreviewPanel.IsPreviewing())
            {
                DrawFontInfoTable(fontBuilder, m3Styles);
            }
        }
        ImGui::EndChild();
    }
}

void UI::FontBuilderPanel::DrawFontInfoTable(const FontBuilder &fontBuilder, const ImGuiEx::M3::M3Styles &m3Styles)
{
    using Spacing      = ImGuiEx::M3::Spacing;
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    if (!fontBuilder.IsBuilding())
    {
        return;
    }
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color<ImGuiCol_Border>(m3Styles.Colors().at(SurfaceToken::outlineVariant))
        .Color<ImGuiCol_TableRowBg>(m3Styles.Colors().at(SurfaceToken::surface))
        .Color<ImGuiCol_TableRowBgAlt>(m3Styles.Colors().at(SurfaceToken::surface))
        .Style<ImGuiStyleVar_ScrollbarSize>(m3Styles[Spacing::XS]);
    ImGui::Indent(m3Styles[Spacing::L]);
    if (ImGui::BeginTable(
            "BasicFontInfo", 2, ImGuiEx::TableFlags().BordersInnerH().ScrollY().NoBordersInBody().SizingFixedFit()
        ))
    {
        auto &names = fontBuilder.GetBaseFont().GetFontNames();
        auto &paths = fontBuilder.GetBaseFont().GetFontPathList();
        if (names.size() == paths.size())
        {
            ImGui::PushFont(nullptr, m3Styles.TitleText().textSize);
            const auto col1LineHeight = m3Styles.TitleText().textSize + m3Styles.SmallLabelText().textSize;
            for (size_t idx = 0; idx < names.size(); idx++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGuiEx::M3::LineTextUnformatted(std::format("{}", idx + 1), col1LineHeight);

                ImGui::TableNextColumn();

                ImGui::Text("%s", names.at(idx).c_str());
                {
                    ImGuiEx::StyleGuard styleGuard1;
                    styleGuard1.Color<ImGuiCol_Text>(m3Styles.Colors().at(ContentToken::onSurfaceVariant));
                    ImGui::PushFont(nullptr, m3Styles.SmallLabelText().textSize);
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

void UI::FontBuilderPanel::DrawToolBar(
    FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles
)
{
    if (ImGuiEx::M3::BeginDockedToolbar(
            m3Styles.GetPixels(M3Spec::SmallIconButton::size), 5, ImGuiEx::M3::SurfaceToken::surfaceContainer, m3Styles
        ))
    {
        DrawToolBarButtons(fontBuilder, settings, m3Styles);
        ImGuiEx::M3::EndDockedToolbar();
    }

    DrawHelpModal();
    DrawWarningsModal();
}

void UI::FontBuilderPanel::DrawToolBarButtons(
    FontBuilder &fontBuilder, Settings &settings, const ImGuiEx::M3::M3Styles &m3Styles
)
{
    ImGui::BeginDisabled(!fontBuilder.IsBuilding());
    auto Button = [&m3Styles](const std::string_view &icon) -> bool {
        return ImGuiEx::M3::IconButtonXS(
            icon, m3Styles, ImGuiEx::M3::SurfaceToken::surfaceContainer, ImGuiEx::M3::ContentToken::onSurfaceVariant
        );
    };

    if (ImGuiEx::M3::IconButtonXS(
            ICON_FA_WRENCH, m3Styles, ImGuiEx::M3::SurfaceToken::primary, ImGuiEx::M3::ContentToken::onPrimary
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
    if (ImGuiEx::M3::IconButtonXS(
            ICON_MD_ALERT_CIRCLE_OUTLINE,
            m3Styles,
            ImGuiEx::M3::SurfaceToken::tertiaryContainer,
            ImGuiEx::M3::ContentToken::onTertiaryContainer
        ))
    {
        centerPopup(TITLE_WARNING);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Warning"), m3Styles);
    ImGui::SameLine();
    if (ImGuiEx::M3::IconButtonXS(
            ICON_MD_HELP_CIRCLE_OUTLINE,
            m3Styles,
            ImGuiEx::M3::SurfaceToken::secondaryContainer,
            ImGuiEx::M3::ContentToken::onSecondaryContainer
        ))
    {
        centerPopup(TITLE_HELP);
    }
    ImGuiEx::M3::SetItemToolTip(Translate("Settings.FontBuilder.Help"), m3Styles);
}

void UI::FontBuilderPanel::DrawHelpModal()
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

void UI::FontBuilderPanel::DrawWarningsModal()
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

} // namespace Ime
