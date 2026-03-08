//
// Created by jamie on 2025/5/21.
//
#pragma once

#include "imgui.h"

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
    static constexpr std::string_view ICON_FILE               = "simple-ime-icons.ttf";
    static constexpr float            MIN_FONT_SIZE_SCALE     = 0.1F;
    static constexpr float            MAX_FONT_SIZE_SCALE     = 5.0F;

    //! Shortcut: support combination of Ctrl, Shift, Alt and a normal key. e.g. "ctrl+shift+f1", "alt+f2", "f3"...
    //! The named key is can't combine by bitwise operation, g.g. "F2 | A" will become to "F3".
    ImGuiKeyChord shortcut;
    bool          enableMod = true;
    bool          enableTsf = true;

    struct Logging
    {
        spdlog::level::level_enum level;
        spdlog::level::level_enum flushLevel;

        bool operator==(const Logging &other) const = default;
    } logging;

    struct Resources
    {
        std::string              translationDir;
        std::vector<std::string> fontPathList;

        bool operator==(const Resources &other) const = default;
    } resources;

    struct Appearance
    {
        uint32_t    themeSourceColor;
        double      themeContrastLevel;
        bool        themeDarkMode;
        std::string language;
        float       zoom;
        int         errorDisplayDuration;
        bool        showSettings;

        bool operator==(const Appearance &other) const = default;
    } appearance;

    struct Input
    {
        bool                  enableUnicodePaste;
        bool                  keepImeOpen;
        WindowPosUpdatePolicy posUpdatePolicy;

        bool operator==(const Input &other) const = default;
    } input;

    bool operator==(const Settings &other) const = default;
};

inline auto GetDefaultSettings() -> Settings
{
    return {
        .shortcut  = ImGuiKey_F2,
        .enableMod = true,
        .enableTsf = true,
        .logging   = {.level = spdlog::level::info, .flushLevel = spdlog::level::info},
        .resources = {.translationDir = "Data/interface/SimpleIME", .fontPathList = {"C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/seguiemj.ttf"}},
        .appearance =
            {.themeSourceColor     = 0xFF673AB7,
                      .themeContrastLevel   = 0.0,
                      .themeDarkMode        = true,
                      .language             = "english",
                      .zoom                 = 1.0,
                      .errorDisplayDuration = 10,
                      .showSettings         = false},
        .input = {.enableUnicodePaste = true, .keepImeOpen = false, .posUpdatePolicy = Settings::WindowPosUpdatePolicy::BASED_ON_CARET}
    };
}

} // namespace Ime
