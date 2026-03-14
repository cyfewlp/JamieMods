//
// Created by jamie on 2026/2/6.
//

#pragma once

#include "imgui.h"

#include <vector>

namespace Ime
{
struct LangProfile;

namespace LanguageBar
{
// FIXME: moveout from ToolWindow?
// we only want lock game and input events when SettingsWindow is showing.
auto Draw(bool &pinned, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> bool;
}; // namespace LanguageBar

} // namespace Ime
