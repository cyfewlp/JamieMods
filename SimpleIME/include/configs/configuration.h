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
    Logging     logging;
    Resources   resources;
    Appearance  appearance;
    Input       input;
};

} // namespace Ime
