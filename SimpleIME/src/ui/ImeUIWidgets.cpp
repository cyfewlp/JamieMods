//
// Created by jamie on 2025/3/7.
//
#include "ui/ImeUIWidgets.h"

#include "ImeWnd.hpp"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "imgui.h"
#include "tsf/LangProfileUtil.h"

#include <format>
#include <string>
#include <utility>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
void ImeUIWidgets::RenderInputMethodChooseWidget(LangProfileUtil *pLangProfileUtil, const ImeWnd *pImeWnd)
{
    auto       &activatedGuid     = pLangProfileUtil->GetActivatedLangProfile();
    auto       &installedProfiles = pLangProfileUtil->GetLangProfiles();
    auto       &profile           = installedProfiles[activatedGuid];
    const char *previewImeName    = profile.desc.c_str();
    if (ImGui::BeginCombo("###InstalledIME", previewImeName))
    {
        uint32_t idx = 0;
        for (const std::pair<GUID, LangProfile> pair : installedProfiles)
        {
            const auto &langProfile = pair.second;
            const bool  isSelected  = langProfile.guidProfile == activatedGuid;
            auto        label       = std::format("{}##{}", langProfile.desc, idx);
            if (ImGui::Selectable(label.c_str()))
            {
                pImeWnd->SendMessageToIme(CM_ACTIVATE_PROFILE, 0, reinterpret_cast<LPARAM>(&pair.first));
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

void ImeUIWidgets::StateWidget(String label, const bool isEnabled) const
{
    const auto *name = m_translation->Get(label);
    if (isEnabled)
    {
        ImGui::Text("%s: %s", name, "\xe2\x9c\x85"); // green ✅
    }
    else
    {
        ImGui::Text("%s: %s", name, "\xe2\x9d\x8c"); // red  ❌
    }
    TrySetItemTooltip(label);
}

void ImeUIWidgets::Button(String label, OnClick onClick) const
{
    if (const auto *name = m_translation->Get(label); ImGui::Button(name))
    {
        onClick();
    }
    TrySetItemTooltip(label);
}

void ImeUIWidgets::Checkbox(String label, bool &checked, const std::function<void(bool isChecked)> &&onChecked) const
{
    const auto *name = m_translation->Get(label);
    if (ImGui::Checkbox(name, &checked))
    {
        onChecked(checked);
    }
    TrySetItemTooltip(label);
}

auto ImeUIWidgets::Checkbox(String label, bool &checked) const -> bool
{
    const auto *name    = m_translation->Get(label);
    bool        clicked = false;
    if (ImGui::Checkbox(name, &checked))
    {
        clicked = true;
    }
    TrySetItemTooltip(label);
    return clicked;
}

auto ImeUIWidgets::Begin(String windowName, bool *open, ImGuiWindowFlags flags) const -> bool
{
    const auto *name = m_translation->Get(windowName);
    m_translation->UseSection(std::string(windowName).erase(0, 1).c_str());
    return ImGui::Begin(name, open, flags);
}

auto ImeUIWidgets::Combo(
    String label, const std::vector<std::string> &values, std::function<bool(const std::string &)> onApply
) -> uint32_t
{
    const auto *name     = m_translation->Get(label);
    uint32_t   &selected = m_uiUint32Vars.at(label);
    const auto *preview  = values.empty() ? "" : values[selected].c_str();

    static bool fShowErrorMessage = false;
    ImGui::BeginGroup();
    if (ImGui::BeginCombo(name, preview))
    {
        uint32_t idx = 0;
        for (const auto &selectableName : values)
        {
            const bool isSelected = selected == idx;
            if (ImGui::Selectable(selectableName.c_str(), isSelected))
            {
                selected = idx;
                if (!isSelected && !onApply(selectableName))
                {
                    fShowErrorMessage = true;
                }
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            idx++;
        }
        ImGui::EndCombo();
    }
    ImGui::EndGroup();
    TrySetItemTooltip(label);

    if (fShowErrorMessage)
    {
        ImGui::Text("Unable to apply %s %s", name, values[selected].c_str());
        ImGui::SameLine();
        if (ImGui::Button(std::format("x##Cancel{}Error", name).c_str()))
        {
            fShowErrorMessage = false;
        }
    }
    SetUInt32Var(label, selected);
    return selected;
}

auto ImeUIWidgets::TrySetItemTooltip(String label) const -> void
{
    auto tooltipKey = std::format("{}_Tooltip", label);
    if (m_translation->Has(tooltipKey.c_str()))
    {
        ImGui::SetItemTooltip("%s", m_translation->Get(tooltipKey.c_str()));
    }
}

auto ImeUIWidgets::SetUInt32Var(String name, uint32_t value) -> void
{
    m_uiUint32Vars.insert_or_assign(name, value);
}

auto ImeUIWidgets::GetUInt32Var(String name) -> std::optional<uint32_t>
{
    return m_uiUint32Vars.contains(name) ? std::optional{m_uiUint32Vars.at(name)} : std::nullopt;
}

auto ImeUIWidgets::GetBoolVar(String name) const -> bool
{
    return m_uiBoolVars.contains(name) ? m_uiBoolVars.at(name) : false;
}
}
}