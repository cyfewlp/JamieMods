//
// Created by jamie on 2025/5/3.
//

#ifndef FONTSIZE_H
#define FONTSIZE_H

#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
struct Config
{
    static constexpr float       FONT_SIZE_TEXT_SMALL    = 12.0F;
    static constexpr float       FONT_SIZE_TEXT          = 14.0F;
    static constexpr float       FONT_SIZE_TITLE_1       = 36.0F;
    static constexpr float       FONT_SIZE_TITLE_2       = 24.0F;
    static constexpr float       FONT_SIZE_TITLE_3       = 18.0F;
    static constexpr float       FONT_SIZE_TITLE_4       = 16.0F;
    static constexpr const char *DEFAULT_FONT            = R"(C:\Windows\Fonts\simsun.ttc)";
    static constexpr const char *DEFAULT_EMOJI_FONT      = R"(C:\Windows\Fonts\seguiemj.ttf)";
    static constexpr const char *IMGUI_INI_FILE_TEMPLATE = R"(Data\interface\{}\imgui.ini)";

    static inline bool INCLUDE_TEMPLATE_ARMOR = true;
    static inline bool SHOW_FAVORITE_OUTFITS  = false;
};
}

#endif // FONTSIZE_H