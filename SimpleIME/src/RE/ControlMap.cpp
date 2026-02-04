//
// Created by jamie on 2026/2/5.
//

#include "RE/ControlMap.h"

#include "common/log.h"

namespace Ime
{

auto ControlMap::SKSE_AllowTextInput(bool allow) -> uint8_t
{
    if (REL::Module::IsSE())
    {
        DoAllowTextInput(allow, textEntryCount);
        return textEntryCount;
    }
    DoAllowTextInput(allow, allowTextInput);
    return allowTextInput;
}

auto ControlMap::GetTextEntryCount() const -> uint8_t
{
    if (REL::Module::IsSE())
    {
        return textEntryCount;
    }
    return allowTextInput;
}

auto ControlMap::GetSingleton() -> ControlMap *
{
    static REL::Relocation<ControlMap **> const singleton{RELOCATION_ID(514705, 400863)};
    return *singleton;
}

void ControlMap::DoAllowTextInput(bool allow, std::uint8_t &entryCount)
{
    if (allow)
    {
        if (entryCount == UINT8_MAX)
        {
            logger::warn("InputManager::AllowTextInput: counter overflow");
        }
        else
        {
            entryCount++;
        }
    }
    else
    {
        if (entryCount == 0)
        {
            logger::warn("InputManager::AllowTextInput: counter underflow");
        }
        else
        {
            entryCount--;
        }
    }

    logger::debug("{} text input, count = {}", allow ? "allowed" : "disallowed", entryCount);
}
} // namespace Ime
