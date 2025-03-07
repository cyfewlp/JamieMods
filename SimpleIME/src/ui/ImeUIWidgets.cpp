//
// Created by jamie on 2025/3/7.
//
#include "ui/ImeUIWidgets.h"

#include "configs/CustomMessage.h"
#include "context.h"
#include "tsf/LangProfileUtil.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        void ImeUIWidgets::RenderInputMethodChooseWidget(LangProfileUtil *pLangProfileUtil, const ImeWnd *pImeWnd)
        {
            auto        activatedGuid     = pLangProfileUtil->GetActivatedLangProfile();
            auto        installedProfiles = pLangProfileUtil->GetLangProfiles();
            auto       &profile           = installedProfiles[activatedGuid];
            const char *previewImeName    = profile.desc.c_str();
            if (ImGui::BeginCombo("###InstalledIME", previewImeName))
            {
                uint32_t idx = 0;
                for (const std::pair<GUID, LangProfile> pair : installedProfiles)
                {
                    auto       langProfile = pair.second;
                    bool const isSelected  = langProfile.guidProfile == activatedGuid;
                    auto       label       = std::format("{}##{}", langProfile.desc, idx);
                    if (ImGui::Selectable(label.c_str()))
                    {
                        pImeWnd->SendMessage_(CM_ACTIVATE_PROFILE, 0, reinterpret_cast<LPARAM>(&pair.first));
                        activatedGuid = pLangProfileUtil->GetActivatedLangProfile();
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                    idx++;
                }
                ImGui::EndCombo();
            }
        }

        void ImeUIWidgets::RenderEnableModWidget(std::function<bool(bool)> &&onEnable)
        {
            static bool EnableModFail = false;
            static bool EnableMod     = true;
            if (ImGui::Checkbox("Enable Mod", &EnableMod))
            {
                if (!onEnable(EnableMod))
                {
                    EnableMod     = false;
                    EnableModFail = true;
                    log_debug("Unable to enable mod: {}", GetLastError());
                }
            }
            ImGui::SetItemTooltip("Uncheck will disable all mod feature(Disable keyboard).");
            if (EnableModFail)
            {
                ImGui::TextColored(ImVec4(1.0f, .0f, .0f, 1.0f), "%s", "Failed to enable mod");
                if (ImGui::Button("x"))
                {
                    EnableModFail = false;
                }
            }
        }

        void ImeUIWidgets::RenderImeStateWidget(const bool isDisabled)
        {
            if (isDisabled)
            {
                ImGui::Text("Ime Enabled %s", "\xe2\x9d\x8c"); // red  ❌
            }
            else
            {
                ImGui::Text("Ime Enabled %s", "\xe2\x9c\x85"); // green ✅
            }
        }

        void ImeUIWidgets::RenderImeFocusStateWidget(const bool isFocused)
        {
            ImGui::Text("Ime Focus: %s", isFocused ? "\xe2\x9c\x85" : "\xe2\x9d\x8c");
            ImGui::SetItemTooltip("Mod must has keyboard focus to work.");
        }

        void ImeUIWidgets::RenderKeepImeOpenWidget(const std::function<bool(bool keepImeOpen)> &&onChecked)
        {
            static bool fKeepImeOpen = Context::GetInstance()->KeepImeOpen();
            if (ImGui::Checkbox("Keep Ime Open", &fKeepImeOpen))
            {
                Context::GetInstance()->SetFKeepImeOpen(fKeepImeOpen);
                if (!onChecked(fKeepImeOpen))
                {
                    log_error("Handle Keep Ime Open failed");
                }
            }
            ImGui::SetItemTooltip("A stupid patch. Use this to fix when Ime disabled in any text entry.");
        }

        void ImeUIWidgets::RenderThemeChooseWidget(const std::vector<std::string> &themeNames, uint32_t &selectedTheme,
                                                   std::function<bool(const std::string &)> &&onApply)
        {
            auto preview = themeNames.empty() ? "" : themeNames[selectedTheme].c_str();
            if (ImGui::BeginCombo("Themes", preview))
            {
                uint32_t idx = 0;
                for (const auto &themeName : themeNames)
                {
                    bool isSelected = selectedTheme == idx;
                    if (ImGui::Selectable(themeName.c_str(), isSelected))
                    {
                        selectedTheme = idx;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                    idx++;
                }
                ImGui::EndCombo();
            }
            static bool fShowErrorMessage = false;
            if (ImGui::Button("Apply"))
            {
                auto &name = themeNames[selectedTheme];
                if (!onApply(name))
                {
                    fShowErrorMessage = true;
                }
            }
            if (fShowErrorMessage)
            {
                ImGui::Text("Unable to apply theme %s", themeNames[selectedTheme].c_str());
                ImGui::SameLine();
                if (ImGui::Button("x##CancelThemeLoadError"))
                {
                    fShowErrorMessage = false;
                }
            }
        }
    }
}