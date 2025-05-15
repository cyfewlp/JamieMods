//
// Created by jamie on 2025/5/3.
//

#ifndef FONTSIZE_H
#define FONTSIZE_H

#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Setting
{
struct UiSetting
{
    static constexpr std::string_view     SETTING_NAME         = "UiSetting";
    static constexpr float                FONT_SCALE_MIN       = 0.5F;
    static constexpr float                FONT_SCALE_MAX       = 5.0F;
    static constexpr float                FONT_SIZE_TEXT_SMALL = 12.0F;
    static constexpr float                FONT_SIZE_TEXT       = 14.0F;
    static constexpr float                FONT_SIZE_TITLE_1    = 36.0F;
    static constexpr float                FONT_SIZE_TITLE_2    = 24.0F;
    static constexpr float                FONT_SIZE_TITLE_3    = 18.0F;
    static constexpr float                FONT_SIZE_TITLE_4    = 16.0F;
    static constexpr std::array<float, 2> ICON_PADDING         = {5.0F, 5.0F};
    static constexpr std::array<float, 2> TABLE_ROW_PADDING    = {3.0F, 3.0F};
    static constexpr const char          *ICON_FONT            = "SymbolsNerdFontMono-Regular.ttf";

    enum DefaultThemeIndex
    {
        DefaultThemeIndex_Classic = -1,
        DefaultThemeIndex_Dark    = -2,
        DefaultThemeIndex_Light   = -3,
        DefaultThemeIndex_Invalid = -4,
    };

    int32_t selectedThemeIndex   = DefaultThemeIndex_Invalid; // setting key: selectedThemeIndex
    float   globalFontScale      = 1.0F;                      // setting key: globalFontScale
    bool    includeTemplateArmor = true;
    bool    showFavoriteOutfits  = false;

    void Reset()
    {
        selectedThemeIndex   = DefaultThemeIndex_Invalid;
        globalFontScale      = 1.0F;
        includeTemplateArmor = true;
        showFavoriteOutfits  = false;
    }

    static auto GetInstance() -> UiSetting *;
};
}
}

#endif // FONTSIZE_H