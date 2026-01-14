//
// Created by jamie on 2025/5/21.
//
#pragma once

#include "common/config.h"

#include <algorithm>
#include <cstdint>
#include <spdlog/common.h>
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
        BASED_ON_CARET
    };
    static constexpr std::string_view ICON_FILE           = "SymbolsNerdFontMono-Regular.ttf";
    static constexpr float            MIN_FONT_SIZE_SCALE = 0.1F;
    static constexpr float            MAX_FONT_SIZE_SCALE = 5.0F;

    int   fontSizeTemp   = 16;    // not persist
    float dpiScale       = 1.0f;  // not persist
    bool  wantResizeFont = false; // not persist

    uint32_t shortcutKey = 0x3C;
    bool     enableMod   = true;
    bool     enableTsf   = true;

    struct
    {
        spdlog::level::level_enum level      = spdlog::level::info;
        spdlog::level::level_enum flushLevel = spdlog::level::info;
    } logging;

    struct
    {
        std::string mainFontPath   = "C:/Windows/Fonts/simsun.ttc";
        std::string emojiFontPath  = "C:/Windows/Fonts/seguiemj.ttf";
        std::string translationDir = "Data/interface/SimpleIME";
    } resources;

    struct
    {
        std::string theme                = "darcula";
        std::size_t themeIndex           = 0; // not persist
        std::string language             = "english";
        float       fontSize             = 16.0F; // not persist
        float       fontSizeScale        = 1.0F;
        int         errorDisplayDuration = 10;
        bool        showSettings         = false;
    } appearance;

    struct
    {
        bool                  enableUnicodePaste = true;
        bool                  keepImeOpen        = false;
        WindowPosUpdatePolicy posUpdatePolicy    = WindowPosUpdatePolicy::BASED_ON_CARET;
    } input;
};
}
}
