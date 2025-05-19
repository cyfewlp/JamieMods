//
// Created by jamie on 2025/5/15.
//

#include "AboutPopup.h"

#include "../UiSettings.h"
#include "common/imgui/ImGuiScope.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Popup
{
void AboutPopup::DoDraw(SosUiData &, bool &)
{
    const auto *plugin = SKSE::PluginDeclaration::GetSingleton();
    {
        auto        fontSize = ImGuiScope::FontSize(Settings::UiSettings::GetInstance()->Title2PxSize());
        const auto &version  = plugin->GetVersion();
        ImGui::Text("%s v%u.%u.%u", plugin->GetName().data(), version.major(), version.minor(), version.patch());
    }
    ImGui::PushFontSize(Settings::UiSettings::GetInstance()->TextSmallPxSize());
    ImGui::Text("A Extra GUI for SkyrimOutfitSystemRE");
    ImGui::PopFontSize();

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