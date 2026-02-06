//
// Created by jamie on 2026/2/6.
//

#pragma once

#include "tsf/LangProfile.h"

#include <unordered_map>

namespace Ime
{

namespace LanguageBar
{

enum State : std::uint8_t
{
    NONE          = 0,
    OPEN_SETTINGS = 0x1,
    PINNED        = 0x2,
    SHOWING       = 0x4,
};

constexpr auto IsPinned(State state) -> bool
{
    return (state & PINNED) != 0;
}

constexpr auto IsShowing(State state) -> bool
{
    return (state & SHOWING) != 0;
}

constexpr auto IsOpenSettings(State state) -> bool
{
    return (state & OPEN_SETTINGS) != 0;
}

auto Draw(bool wantToggle, const GUID &activeLangGuid, const std::unordered_map<GUID, LangProfile> &langProfiles)
    -> State;

} // namespace LanguageBar

} // namespace Ime
