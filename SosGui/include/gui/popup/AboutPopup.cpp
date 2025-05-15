//
// Created by jamie on 2025/5/15.
//

#include "AboutPopup.h"

#include "../UiSetting.h"
#include "common/imgui/ImGuiScope.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Popup
{
void AboutPopup::DoDraw(SosUiData &, bool &)
{
    const auto *plugin = SKSE::PluginDeclaration::GetSingleton();
    {
        auto        fontSize = ImGuiScope::FontSize(Setting::UiSetting::FONT_SIZE_TITLE_2);
        const auto &version  = plugin->GetVersion();
        ImGui::Text("%s v%u.%u.%u", plugin->GetName().data(), version.major(), version.minor(), version.patch());
    }
    ImGui::Text("A Extra GUI for SkyrimOutfitSystemRE");

    ImGui::Spacing();
    ImGui::Text("%-10s %s", "Build Date", __DATE__);
    ImGui::Text("%-10s %s", "Author", plugin->GetAuthor().data());
    ImGui::Text("%-10s ", "Github");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("https://github.com/cyfewlp/JamieMods");
    ImGui::Text("%-10s ", "Nexus");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("https://next.nexusmods.com/profile/JamieYin101");
}
}
}