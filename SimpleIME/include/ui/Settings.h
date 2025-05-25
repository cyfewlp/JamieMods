//
// Created by jamie on 2025/5/21.
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include "common/config.h"

#include <cstdint>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
struct Settings
{
    static constexpr std::string_view IME_MESSAGE_FAKE_MENU = "ImeMessageFakeMenu";

    enum class FocusType : std::uint16_t
    {
        Permanent = 0,
        Temporary
    };

    enum class WindowPosUpdatePolicy : std::uint16_t
    {
        NONE = 0,
        BASED_ON_CURSOR,
        BASED_ON_CARET,
    };

    std::string           theme                 = "";
    std::string           language              = "english";
    WindowPosUpdatePolicy windowPosUpdatePolicy = WindowPosUpdatePolicy::NONE;
    FocusType             focusType             = FocusType::Permanent;
    float                 fontSizeScale         = 1.0F;
    bool                  enableUnicodePaste    = true;
    bool                  showSettings          = false;
    bool                  keepImeOpen           = false;
    bool                  enableMod             = false;
};
}
}

#endif // SETTINGS_H
