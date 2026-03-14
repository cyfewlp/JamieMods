//
// Created by jamie on 2026/3/8.
//

#pragma once

#include <string>
#include <vector>

namespace Ime
{
//! @brief Temp struct to hold the configuration data loaded from or to be saved to the configuration file.
//! This struct is designed to be easily serializable and deserializable to and from the configuration file,
//! and should not contain any logic or data that is not directly related to the configuration file.
struct Configuration
{
    static constexpr std::uint32_t INVALID_COLOR = UINT_MAX;

    struct Logging
    {
        std::string level;
        std::string flushLevel;
    };

    struct Resources
    {
        std::string              translationDir;
        std::vector<std::string> fontPathList;
    };

    struct Appearance
    {
        uint32_t    themeSourceColor;
        double      themeContrastLevel;
        bool        themeDarkMode;
        std::string language;
        float       zoom;
        int         errorDisplayDuration;
        bool        showSettings;
    };

    struct Input
    {
        bool        enableUnicodePaste;
        bool        keepImeOpen;
        std::string posUpdatePolicy;
    };

    std::string shortcut;
    bool        enableMod;
    bool        enableTsf;
    bool        fixInconsistentTextEntryCount;
    Logging     logging;
    Resources   resources;
    Appearance  appearance;
    Input       input;
};

// The default configuration values intentionally set to invalid and only promise some important scalar value valid.
// These invalid value should be overridden by settings_converter later.
constexpr auto GetDefaultConfiguration() -> Configuration
{
    return Configuration{
        .shortcut                      = "",
        .enableMod                     = true,
        .enableTsf                     = true,
        .fixInconsistentTextEntryCount = true,
        .logging                       = {.level = "", .flushLevel = ""},
        .resources                     = {.translationDir = "", .fontPathList = {}},
        .appearance =
            {.themeSourceColor     = Configuration::INVALID_COLOR,
                                          .themeContrastLevel   = 0.0,
                                          .themeDarkMode        = true,
                                          .language             = "",
                                          .zoom                 = -1.0F,
                                          .errorDisplayDuration = 10,
                                          .showSettings         = false},
        .input = {.enableUnicodePaste = true, .keepImeOpen = false, .posUpdatePolicy = ""}
    };
}

} // namespace Ime
