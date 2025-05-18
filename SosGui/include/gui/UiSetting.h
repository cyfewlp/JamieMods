//
// Created by jamie on 2025/5/3.
//

#ifndef FONTSIZE_H
#define FONTSIZE_H

#include "common/config.h"
#include "font/FontInfo.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Setting
{
struct UiSetting
{
    static constexpr std::string_view     SETTING_NAME       = "UiSetting";
    static constexpr int                  FONT_POINT_SIZE    = 14;
    static constexpr int                  FONT_PT_TEXT_SMALL = 12;
    static constexpr int                  FONT_PT_TEXT       = 14;
    static constexpr int                  FONT_PT_TITLE_1    = 36;
    static constexpr int                  FONT_PT_TITLE_2    = 24;
    static constexpr int                  FONT_PT_TITLE_3    = 18;
    static constexpr int                  FONT_PT_TITLE_4    = 16;
    static constexpr float                FONT_SCALE_MIN     = 0.5F;
    static constexpr float                FONT_SCALE_MAX     = 5.0F;
    static constexpr std::array<float, 2> ICON_PADDING       = {5.0F, 5.0F};
    static constexpr std::array<float, 2> TABLE_ROW_PADDING  = {3.0F, 3.0F};
    static constexpr const char          *ICON_FONT          = "SymbolsNerdFontMono-Regular.ttf";

private:
    UiSetting();

public:
    enum DefaultThemeIndex
    {
        DefaultThemeIndex_Classic = -1,
        DefaultThemeIndex_Dark    = -2,
        DefaultThemeIndex_Light   = -3,
        DefaultThemeIndex_Invalid = -4,
    };

    float FONT_PX_TEXT_SMALL = 0.0F;
    float FONT_PX_TEXT       = 0.0F;
    float FONT_PX_TITLE_1    = 0.0F;
    float FONT_PX_TITLE_2    = 0.0F;
    float FONT_PX_TITLE_3    = 0.0F;
    float FONT_PX_TITLE_4    = 0.0F;

    int32_t  selectedThemeIndex   = DefaultThemeIndex_Invalid; // setting key: selectedThemeIndex
    float    globalFontScale      = 1.0F;                      // setting key: globalFontScale
    bool     includeTemplateArmor = true;
    bool     showFavoriteOutfits  = false;
    FontInfo fontInfo;

    // std::string asciiFontName    = ""; // for ascii + latin character;

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