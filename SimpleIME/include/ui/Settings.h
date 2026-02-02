//
// Created by jamie on 2025/5/21.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <spdlog/common.h>
#include <string>

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
    static constexpr std::string_view DEFAULT_MAIN_FONT_PATH  = "C:/Windows/Fonts/simsun.ttc";
    static constexpr std::string_view DEFAULT_EMOJI_FONT_PATH = "C:/Windows/Fonts/seguiemj.ttf";
    static constexpr std::string_view ICON_FILE               = "SymbolsNerdFontMono-Regular.ttf";
    static constexpr float            MIN_FONT_SIZE_SCALE     = 0.1F;
    static constexpr float            MAX_FONT_SIZE_SCALE     = 5.0F;

    struct RuntimeState
    {
        float fontSize       = 20.F;
        float dpiScale       = 1.0F;
        bool  wantResizeFont = false;

        bool operator==(const RuntimeState &other) const = default;
    } state;

    uint32_t shortcutKey = 0x3C;
    bool     enableMod   = true;
    bool     enableTsf   = true;

    struct Logging
    {
        spdlog::level::level_enum level      = spdlog::level::info;
        spdlog::level::level_enum flushLevel = spdlog::level::info;

        bool operator==(const Logging &other) const = default;
    } logging;

    struct Resources
    {
        std::string translationDir = "Data/interface/SimpleIME";

        std::vector<std::string> fontPathList = {"C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/seguiemj.ttf"};

        bool operator==(const Resources &other) const = default;
    } resources;

    struct Appearance
    {
        uint32_t    themeSourceColor     = 0xFF673AB7;
        bool        themeDarkMode        = true;
        std::string language             = "english";
        float       zoom                 = 1.0F;
        int         errorDisplayDuration = 10;
        bool        showSettings         = false;

        bool operator==(const Appearance &other) const = default;
    } appearance;

    struct Input
    {
        bool                  enableUnicodePaste = true;
        bool                  keepImeOpen        = false;
        WindowPosUpdatePolicy posUpdatePolicy    = WindowPosUpdatePolicy::BASED_ON_CARET;

        bool operator==(const Input &other) const = default;
    } input;

    bool operator==(const Settings &other) const = default;
};
}
