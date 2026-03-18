//
// Created by jamie on 2026/3/8.
//
#include "configs/settings_converter.h"

#include "configs/configuration.h"
#include "imguiex/ErrorNotifier.h"
#include "log.h"
#include "ui/Settings.h"

namespace Ime
{

namespace
{
constexpr std::uint32_t RGB_MASK = 0x00FFFFFF;

// clang-format off
// ReSharper disable All
static const char* const GKeyNames[] =
{
    "tab", "leftarrow", "rightarrow", "uparrow", "downarrow", "pageup", "pagedown",
    "home", "end", "insert", "delete", "backspace", "space", "enter", "escape",
    "leftctrl", "leftshift", "leftalt", "leftsuper", "rightctrl", "rightshift", "rightalt", "rightsuper", "menu",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f", "g", "h",
    "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
    "f13", "f14", "f15", "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23", "f24",
    "apostrophe", "comma", "minus", "period", "slash", "semicolon", "equal", "leftbracket",
    "backslash", "rightbracket", "graveaccent", "capslock", "scrolllock", "numlock", "printscreen",
    "pause", "keypad0", "keypad1", "keypad2", "keypad3", "keypad4", "keypad5", "keypad6",
    "keypad7", "keypad8", "keypad9", "keypaddecimal", "keypaddivide", "keypadmultiply",
    "keypadsubtract", "keypadadd", "keypadenter", "keypadequal",
    "appback", "appforward", "oem102",
    "gamepadstart", "gamepadback",
    "gamepadfaceleft", "gamepadfaceright", "gamepadfaceup", "gamepadfacedown",
    "gamepaddpadleft", "gamepaddpadright", "gamepaddpadup", "gamepaddpaddown",
    "gamepadl1", "gamepadr1", "gamepadl2", "gamepadr2", "gamepadl3", "gamepadr3",
    "gamepadlstickleft", "gamepadlstickright", "gamepadlstickup", "gamepadlstickdown",
    "gamepadrstickleft", "gamepadrstickright", "gamepadrstickup", "gamepadrstickdown",
    "mouseleft", "mouseright", "mousemiddle", "mousex1", "mousex2", "mousewheelx", "mousewheely",
    "modctrl", "modshift", "modalt", "modsuper", // ReservedForModXXX are showing the ModXXX names.
};

// ReSharper restore All
// clang-format on

auto FindKeyByName(std::string_view name) -> std::optional<ImGuiKey>
{
    const size_t len = sizeof(GKeyNames) / sizeof(GKeyNames[0]);
    for (size_t i = 0; i < len; ++i)
    {
        if (name == GKeyNames[i])
        {
            return static_cast<ImGuiKey>(i + ImGuiKey_NamedKey_BEGIN);
        }
    }
    return std::nullopt;
}

auto TryParseKeyWithModifier(std::string_view name) -> std::optional<ImGuiKeyChord>
{
    if (name == "ctrl")
    {
        return ImGuiMod_Ctrl;
    }
    if (name == "shift")
    {
        return ImGuiMod_Shift;
    }
    if (name == "alt")
    {
        return ImGuiMod_Alt;
    }
    if (name == "super")
    {
        return ImGuiMod_Super;
    }
    return std::nullopt;
}

auto CompareString(std::string_view str1, std::string_view str2) -> bool
{
    if (str1.length() != str2.length()) return false;
    return std::ranges::equal(str1, str2, [](char a, char b) {
        return tolower(a) == tolower(b);
    });
}

auto Trim(std::string_view str) -> std::string_view
{
    size_t startPos = str.find_first_not_of(" \t");
    if (startPos == std::string::npos) return ""; // str is all whitespace
    size_t endPos = str.find_last_not_of(" \t");
    return std::string_view(str).substr(startPos, endPos - startPos + 1);
}

template <typename Type>
struct Converter;

template <>
struct Converter<ImGuiKeyChord>
{
    static auto fromString(std::string_view str) -> std::optional<ImGuiKeyChord>
    {
        ImGuiKeyChord shortcut      = 0;
        bool          parseComplete = false;
        // shortcut can't be only modifier key, must have a normal key. e.g. "Ctrl+Shift" is invalid, but "Ctrl+F1" is valid.
        bool          onlyModifier  = true;

        std::string shortcutErasedSpace;
        shortcutErasedSpace.reserve(str.size());
        for (char c : str)
        {
            if (!std::isspace(c))
            {
                shortcutErasedSpace.push_back(static_cast<char>(std::tolower(c)));
            }
        }

        size_t                 startPos              = 0;
        size_t                 partPos               = 0;
        const std::string_view shortcutErasedSpaceSv = shortcutErasedSpace;
        while (startPos < shortcutErasedSpace.size() && partPos != std::string::npos)
        {
            partPos = shortcutErasedSpaceSv.find_first_of("+", startPos);
            const auto keyName =
                (partPos == std::string::npos) ? shortcutErasedSpaceSv.substr(startPos) : shortcutErasedSpaceSv.substr(startPos, partPos - startPos);
            if (const auto modOpt = TryParseKeyWithModifier(keyName); modOpt.has_value())
            {
                startPos = partPos + 1;
                shortcut |= modOpt.value();
                parseComplete = (partPos == std::string::npos);
            }
            else if (const auto foundKeyOpt = FindKeyByName(keyName); foundKeyOpt.has_value())
            {
                if (!onlyModifier) // already contain a named key!
                {
                    parseComplete = false;
                    break;
                }
                startPos     = partPos + 1;
                onlyModifier = false;
                shortcut |= foundKeyOpt.value();
                parseComplete = (partPos == std::string::npos);
            }
        }
        if (parseComplete && !onlyModifier)
        {
            return shortcut;
        }
        return std::nullopt;
    }

    static auto toString(ImGuiKeyChord chord) -> std::optional<std::string>
    {
        std::string result;
        if ((chord & ImGuiMod_Ctrl) == ImGuiMod_Ctrl) result += "ctrl+";
        if ((chord & ImGuiMod_Shift) == ImGuiMod_Shift) result += "shift+";
        if ((chord & ImGuiMod_Alt) == ImGuiMod_Alt) result += "alt+";
        if ((chord & ImGuiMod_Super) == ImGuiMod_Super) result += "super+";

        ImGuiKey key = (ImGuiKey)(chord & ~ImGuiMod_Mask_);
        // Unsupported multiple key!
        if (key >= ImGuiKey_NamedKey_BEGIN && key <= ImGuiKey_NamedKey_END)
        {
            result += GKeyNames[key - ImGuiKey_NamedKey_BEGIN];
        }
        if (!result.empty() && result.back() != '+')
        {
            return result;
        }
        return std::nullopt;
    }
};

template <>
struct Converter<spdlog::level::level_enum>
{
    static auto fromString(std::string_view str) -> std::optional<spdlog::level::level_enum>
    {
        if (CompareString(Trim(str), "trace")) return spdlog::level::trace;
        if (CompareString(Trim(str), "debug")) return spdlog::level::debug;
        if (CompareString(Trim(str), "info")) return spdlog::level::info;
        if (CompareString(Trim(str), "warn")) return spdlog::level::warn;
        if (CompareString(Trim(str), "err")) return spdlog::level::err;
        if (CompareString(Trim(str), "critical")) return spdlog::level::critical;
        if (CompareString(Trim(str), "off")) return spdlog::level::off;
        return std::nullopt;
    }

    // clang-format off
    static auto toString(spdlog::level::level_enum level) -> std::optional<std::string>
    {
        switch (level)
        {
            case spdlog::level::trace: return "trace";
            case spdlog::level::debug: return "debug";
            case spdlog::level::info: return "info";
            case spdlog::level::warn: return "warn";
            case spdlog::level::err: return "err";
            case spdlog::level::critical: return "critical";
            case spdlog::level::off: return "off";
            default: return std::nullopt;
        }
        return std::nullopt;
    }

    // clang-format on
};

template <>
struct Converter<Settings::WindowPosUpdatePolicy>
{
    static auto fromString(std::string_view str) -> std::optional<Settings::WindowPosUpdatePolicy>
    {
        if (CompareString(Trim(str), "none")) return Settings::WindowPosUpdatePolicy::NONE;
        if (CompareString(Trim(str), "based_on_caret")) return Settings::WindowPosUpdatePolicy::BASED_ON_CARET;
        if (CompareString(Trim(str), "based_on_cursor")) return Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR;

        return std::nullopt;
    }

    static auto toString(Settings::WindowPosUpdatePolicy policy) -> std::optional<std::string>
    {
        switch (policy)
        {
            case Settings::WindowPosUpdatePolicy::BASED_ON_CARET:
                return "based_on_caret";
            case Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR:
                return "based_on_cursor";
            case Settings::WindowPosUpdatePolicy::NONE:
                return "none";
            default:
                return std::nullopt;
        }
    }
};

} // namespace

auto ConvertConfigurationToSettings(const Configuration &config) -> Settings
{
    Settings settings                      = GetDefaultSettings();
    settings.enableMod                     = config.enableMod;
    settings.enableTsf                     = config.enableTsf;
    settings.fixInconsistentTextEntryCount = config.fixInconsistentTextEntryCount;

    if (const auto shortCut = Converter<ImGuiKeyChord>::fromString(config.shortcut); shortCut.has_value())
    {
        settings.shortcut = shortCut.value();
    }

    if (const auto levelOpt = Converter<spdlog::level::level_enum>::fromString(config.logging.level); levelOpt.has_value())
    {
        settings.logging.level = levelOpt.value();
    }

    if (const auto levelOpt = Converter<spdlog::level::level_enum>::fromString(config.logging.flushLevel); levelOpt.has_value())
    {
        settings.logging.flushLevel = levelOpt.value();
    }

    // Resources
    if (!config.resources.translationDir.empty())
    {
        settings.resources.translationDir = config.resources.translationDir;
    }
    if (!config.resources.fontPathList.empty())
    {
        settings.resources.fontPathList = config.resources.fontPathList;
    }

    // Appearance
    if (config.appearance.themeSourceColor != UINT_MAX)
    {
        settings.appearance.schemeConfig.sourceColor = config.appearance.themeSourceColor & RGB_MASK;
    }
    settings.appearance.schemeConfig.contrastLevel = config.appearance.themeContrastLevel;
    settings.appearance.schemeConfig.darkMode      = config.appearance.themeDarkMode;
    if (!config.appearance.language.empty())
    {
        settings.appearance.language = config.appearance.language;
    }
    settings.appearance.zoom                  = config.appearance.zoom;
    settings.appearance.errorDisplayDuration  = config.appearance.errorDisplayDuration;
    settings.appearance.showSettings          = config.appearance.showSettings;
    settings.appearance.verticalCandidateList = config.appearance.verticalCandidateList;

    // Input
    settings.input.enableUnicodePaste = config.input.enableUnicodePaste;
    settings.input.keepImeOpen        = config.input.keepImeOpen;

    if (const auto posPolicyOpt = Converter<Settings::WindowPosUpdatePolicy>::fromString(config.input.posUpdatePolicy); posPolicyOpt.has_value())
    {
        settings.input.posUpdatePolicy = posPolicyOpt.value();
    }

    return settings;
}

auto ConvertSettingsToConfiguration(const Settings &settings) -> Configuration
{
    Configuration configuration{};
    configuration.enableMod                     = settings.enableMod;
    configuration.enableTsf                     = settings.enableTsf;
    configuration.fixInconsistentTextEntryCount = settings.fixInconsistentTextEntryCount;

    ErrorNotifier errorNotifier = ErrorNotifier::GetInstance();
    if (const auto shortcut = Converter<ImGuiKeyChord>::toString(settings.shortcut); shortcut.has_value())
    {
        configuration.shortcut = shortcut.value();
    }
    else
    {
        configuration.shortcut = "f2";

        errorNotifier.Error("Config parse error: Convert shortcut key to string failed, fallback to default value: F2");
    }

    if (const auto levelOpt = Converter<spdlog::level::level_enum>::toString(settings.logging.level); levelOpt.has_value())
    {
        configuration.logging.level = levelOpt.value();
    }
    else
    {
        configuration.logging.level = "info";
        errorNotifier.Error("Config parse error: Convert log level to string failed, fallback to default value: info");
    }

    if (const auto levelOpt = Converter<spdlog::level::level_enum>::toString(settings.logging.flushLevel); levelOpt.has_value())
    {
        configuration.logging.flushLevel = levelOpt.value();
    }
    else
    {
        configuration.logging.flushLevel = "info";
        errorNotifier.Error("Config parse error: Convert log flush level to string failed, fallback to default value: info");
    }

    // Resources
    configuration.resources.translationDir = settings.resources.translationDir;
    configuration.resources.fontPathList   = settings.resources.fontPathList;

    // Appearance
    configuration.appearance.themeSourceColor      = RGB_MASK & settings.appearance.schemeConfig.sourceColor;
    configuration.appearance.themeContrastLevel    = settings.appearance.schemeConfig.contrastLevel;
    configuration.appearance.themeDarkMode         = settings.appearance.schemeConfig.darkMode;
    configuration.appearance.language              = settings.appearance.language;
    configuration.appearance.zoom                  = settings.appearance.zoom;
    configuration.appearance.errorDisplayDuration  = settings.appearance.errorDisplayDuration;
    configuration.appearance.showSettings          = settings.appearance.showSettings;
    configuration.appearance.verticalCandidateList = settings.appearance.verticalCandidateList;

    // Input
    configuration.input.enableUnicodePaste = settings.input.enableUnicodePaste;
    configuration.input.keepImeOpen        = settings.input.keepImeOpen;

    if (const auto posPolicyOpt = Converter<Settings::WindowPosUpdatePolicy>::toString(settings.input.posUpdatePolicy); posPolicyOpt.has_value())
    {
        configuration.input.posUpdatePolicy = posPolicyOpt.value();
    }
    else
    {
        configuration.input.posUpdatePolicy = "based_on_caret";
        errorNotifier.Error("Config parse error: Convert window position update policy to string failed, fallback to default value: based_on_caret");
    }
    return configuration;
}

} // namespace Ime
