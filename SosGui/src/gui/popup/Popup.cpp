#include "gui/popup/Popup.h"

#include "Translation.h"
#include "gui/UiSettings.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imgui/ImThemeLoader.h"
#include "imguiex/ErrorNotifier.h"
#include "path_utils.h"
#include "util/ImGuiUtil.h"
#include "util/utils.h"

#include <ranges>
#include <string>

namespace SosGui
{
bool Popup::ModalPopup::Draw(SosUiData &uiData, bool &confirmed, ImGuiWindowFlags flags)
{
    confirmed       = false;
    const auto name = Translation::Translate(nameKey.data());
    if (wantOpen)
    {
        wantOpen = false;
        ImGui::OpenPopup(name.c_str(), popupFlags);
    }
    if (!ImGui::BeginPopupModal(name.c_str(), &showPopup, flags))
    {
        return showPopup;
    }
    DoDraw(uiData, confirmed);
    ImGui::EndPopup();
    return showPopup;
}

void Popup::ModalPopup::RenderMultilineMessage(std::string_view message)
{
    constexpr auto delim    = "\\n"sv;
    const auto    *viewport = ImGui::GetMainViewport();
    ImGui::PushTextWrapPos(ImGui::GetCursorScreenPos().x + viewport->WorkSize.x * 0.5F);
    const auto &contentWidth = ImGui::GetContentRegionAvail().x;
    for (const auto &lineView : std::views::split(message, delim))
    {
        auto lineStrView = std::string_view(lineView);
        if (lineStrView.empty())
        {
            continue;
        }
        auto line = std::string(lineStrView);
        if (const auto textSize = ImGui::CalcTextSize(line.data()); contentWidth > textSize.x)
        {
            ImGui::SetCursorPosX((contentWidth - textSize.x) * 0.5F);
        }
        ImGui::Text("%s", line.data());
    }
    ImGui::PopTextWrapPos();
}

void Popup::ModalPopup::RenderConfirmButtons(__out bool &confirmed)
{
    const float contentWidth = ImGui::GetContentRegionAvail().x;
    const auto  quarterWidth = contentWidth * 0.25F;
    ImGui::SetCursorPosX(quarterWidth * 0.5F);
    if (ImGui::Button(Translate1("Yes"), ImVec2(quarterWidth, 0.0F)))
    {
        ConfirmAndClose(confirmed);
    }
    ImGui::SameLine();

    ImGui::SetCursorPosX(quarterWidth * 2.5F);
    if (ImGui::Button(Translate1("No"), ImVec2(quarterWidth, 0.0F)))
    {
        ImGui::CloseCurrentPopup();
    }
}

void Popup::DeleteOutfitPopup::DoDraw(SosUiData &, bool &confirmed)
{
    ImGuiUtil::Text(std::format("{} \"{}\"?", Translate("Panels.OutfitEdit.Delete"), wanDeleteOutfitName));
    RenderConfirmButtons(confirmed);
}

void Popup::DrawSettingsPopup(std::string_view name)
{
    const auto   &loader     = ImTheme::Loader::GetInstance();
    auto         *settings   = Settings::UiSettings::GetInstance();
    const int32_t themeIndex = settings->selectedThemeIndex;
    std::string   preview    = "";
    const auto   &themes     = loader.GetAvailableThemes();
    if (themeIndex > Settings::UiSettings::DefaultThemeIndex_Invalid)
    {
        switch (themeIndex)
        {
            case Settings::UiSettings::DefaultThemeIndex_Classic:
                preview = "Default: Classic";
                break;
            case Settings::UiSettings::DefaultThemeIndex_Dark:
                preview = "Default: Dark";
                break;
            case Settings::UiSettings::DefaultThemeIndex_Light:
                preview = "Default: Light";
                break;
            default: {
                if (loader.IsIndexInRange(themeIndex))
                {
                    preview = themes[themeIndex].name;
                }
                break;
            }
        }
    }
    if (!ImGui::BeginCombo("Theme", preview.c_str()))
    {
        return;
    }

    if (ImGui::Selectable("Default: Classic", false))
    {
        ImGui::StyleColorsClassic();
        settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Classic;
    }

    if (ImGui::Selectable("Default: Dark", false))
    {
        ImGui::StyleColorsDark();
        settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Dark;
    }

    if (ImGui::Selectable("Default: Light", false))
    {
        ImGui::StyleColorsLight();
        settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Light;
    }

    for (size_t index = 0; index < themes.size(); ++index)
    {
        const auto &theme = themes[index];
        if (ImGui::Selectable(theme.name.c_str(), false))
        {
            try
            {
                loader.UseTheme(index, utils::GetInterfaceFile(ImTheme::THEME_FILE_NAME));
                settings->selectedThemeIndex = index;
            }
            catch (ImTheme::Loader::Error &e)
            {
                ErrorNotifier::GetInstance().Error(std::format("Can't use theme {}: {}", theme.name, e.what()));
                settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Invalid;
            }
        }
    }
    ImGui::EndCombo();

    // FIXME: should use ourself setings
    // if (themeIndex != settings->selectedThemeIndex)
    // {
    //     ImGui::MarkIniSettingsDirty();
    // }
}

void Popup::DrawAboutPopup(std::string_view name)
{
    if (ImGui::BeginPopupModal(name.data()))
    {
        ImGuiUtil::Text(name);

        const auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
        const auto &version = plugin->GetVersion();
        ImGuiUtil::Text(std::format("{} v{}.{}.{}", plugin->GetName(), version.major(), version.minor(), version.patch()));

        ImGui::Spacing();
        ImGuiUtil::Text(std::format("{:10} {}", "Build Date", __DATE__));
        ImGuiUtil::Text(std::format("{:10} {}", "Author Date", plugin->GetAuthor()));
        ImGuiUtil::Text(std::format("{:10}", "Github"));
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("https://github.com/cyfewlp/JamieMods");
        ImGuiUtil::Text(std::format("{:10}", "Nexus Mods"));
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("https://next.nexusmods.com/profile/JamieYin101");
        ImGui::EndPopup();
    }
}
} // namespace SosGui
