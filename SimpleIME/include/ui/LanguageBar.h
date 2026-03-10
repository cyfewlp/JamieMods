//
// Created by jamie on 2026/2/6.
//

#pragma once

#include "imgui.h"

#include <vector>

namespace Ime
{
struct LangProfile;

class LanguageBar
{
public:
    auto Draw(bool &pinned, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> bool;

private:
    auto DoDraw(bool &openSettings, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> void;
};

} // namespace Ime
