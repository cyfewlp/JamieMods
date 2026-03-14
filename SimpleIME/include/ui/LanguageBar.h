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
auto Draw(bool &pinned, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> bool;
}; // namespace LanguageBar

} // namespace Ime
