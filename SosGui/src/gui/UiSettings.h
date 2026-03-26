//
// Created by jamie on 2025/5/3.
//
#pragma once

#include "font/FontInfo.h"

namespace SosGui::Settings
{
class UiSettings
{
    UiSettings();

public:
    static constexpr std::string_view     SETTING_NAME       = "UiSetting";
    static constexpr int                  FONT_POINT_SIZE    = 14;
    static constexpr int                  TEXT_SMALL_PT_SIZE = 12;
    static constexpr int                  TEXT_PT_SIZE       = 14;
    static constexpr int                  TITLE_1_PT_SIZE    = 36;
    static constexpr int                  TITLE_2_PT_SIZE    = 24;
    static constexpr int                  TITLE_3_PT_SIZE    = 18;
    static constexpr int                  TITLE_4_PT_SIZE    = 16;
    static constexpr std::array<float, 2> ICON_PADDING       = {5.0F, 5.0F};
    static constexpr std::array<float, 2> TABLE_ROW_PADDING  = {3.0F, 3.0F};
    static constexpr std::string_view     ICON_FONT          = "SymbolsNerdFontMono-Regular.ttf";

    enum DefaultThemeIndex
    {
        DefaultThemeIndex_Classic = -1,
        DefaultThemeIndex_Dark    = -2,
        DefaultThemeIndex_Light   = -3,
        DefaultThemeIndex_Invalid = -4,
    };

    int32_t  selectedThemeIndex   = DefaultThemeIndex_Invalid; // setting key: selectedThemeIndex
    bool     includeTemplateArmor = true;
    bool     showFavoriteOutfits  = false;
    FontInfo fontInfo; // setting key: start with fontinfo

    [[nodiscard]] constexpr float TextSmallPxSize() const { return m_TextSmallPxSize; }

    [[nodiscard]] constexpr float TextPxSize() const { return m_TextPxSize; }

    [[nodiscard]] constexpr float Title1PxSize() const { return m_Title1PxSize; }

    [[nodiscard]] constexpr float Title2PxSize() const { return m_Title2PxSize; }

    [[nodiscard]] constexpr float Title3PxSize() const { return m_Title3PxSize; }

    [[nodiscard]] constexpr float Title4PxSize() const { return m_Title4PxSize; }

    void UpdateFontSize();

    void Reset()
    {
        selectedThemeIndex   = DefaultThemeIndex_Invalid;
        includeTemplateArmor = true;
        showFavoriteOutfits  = false;
    }

    static auto GetInstance() -> UiSettings *
    {
        static UiSettings g_instance;
        return &g_instance;
    }

private:
    float m_TextSmallPxSize = 0.0F;
    float m_TextPxSize      = 0.0F;
    float m_Title1PxSize    = 0.0F;
    float m_Title2PxSize    = 0.0F;
    float m_Title3PxSize    = 0.0F;
    float m_Title4PxSize    = 0.0F;
};
} // namespace SosGui::Settings
