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
    enum class WindowPosUpdatePolicy : std::uint16_t
    {
        NONE = 0,
        BASED_ON_CURSOR,
        BASED_ON_CARET,
    };

    std::string           theme                 = "Invalid";
    std::size_t           themeIndex            = 0; // not persist
    std::string           language              = "english";
    WindowPosUpdatePolicy windowPosUpdatePolicy = WindowPosUpdatePolicy::NONE;
    int                   fontSize              = 16; // not persist
    float                 fontSizeScale         = 1.0F;
    float                 dpiScale              = 1.0f; // not persist
    bool                  wantResizeFont       = false;// not persist
    bool                  enableUnicodePaste    = true;
    bool                  showSettings          = false;
    bool                  keepImeOpen           = false;
    bool                  enableMod             = false;
};
}
}

#endif // SETTINGS_H
