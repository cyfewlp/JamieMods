//
// Created by jamie on 2026/2/6.
//

#pragma once

#include <vector>

namespace Ime
{
struct LangProfile;

namespace UI
{
class ToolWindow;

namespace LanguageBar
{
auto Draw(bool &pinned, bool &toolWindowShowing, const LangProfile &activeLangProfile, const std::vector<LangProfile> &langProfiles) -> void;
}; // namespace LanguageBar
} // namespace UI

} // namespace Ime
