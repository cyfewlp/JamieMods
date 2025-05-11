//
// Created by jamie on 2025/5/3.
//

#ifndef FONTSIZE_H
#define FONTSIZE_H

#include "common/config.h"

namespace
LIBC_NAMESPACE_DECL
{
struct Config
{
    static constexpr float FONT_SIZE_TEXT         = 14.0F;
    static constexpr float FONT_SIZE_TITLE_1      = 36.0F;
    static constexpr float FONT_SIZE_TITLE_2      = 24.0F;
    static constexpr float FONT_SIZE_TITLE_3      = 18.0F;
    static constexpr float FONT_SIZE_TITLE_4      = 16.0F;
    static inline bool     INCLUDE_TEMPLATE_ARMOR = true;
};
}

#endif // FONTSIZE_H