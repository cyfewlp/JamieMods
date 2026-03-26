//
// Created by jamie on 2025/5/15.
//

#include "AboutPopup.h"

#include "gui/UiSettings.h"
#include "imguiex/imguiex_m3.h"

namespace SosGui::Popup
{
void AboutPopup::DoDraw(SosUiData &uiData, bool &)
{
    const auto *plugin = SKSE::PluginDeclaration::GetSingleton();
    ImGuiEx::M3::TextUnformatted("A Extra GUI for SkyrimOutfitSystemRE");
    {
        auto       &m3Styles  = ImGuiEx::M3::Context::GetM3Styles();
        const auto  fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelSmall>();
        const auto &version   = plugin->GetVersion();
        ImGuiEx::M3::TextUnformatted(std::format("{} v{}.{}.{}", plugin->GetName(), version.major(), version.minor(), version.patch()));
    }

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
} // namespace SosGui::Popup
