//
// Created by jamie on 2025/2/7.
//
#include "Configs.h"

void LIBC_NAMESPACE::SimpleIME::AppConfig::of(CSimpleIniA &ini)
{
    eastAsiaFontFile      = ini.GetValue("General", "EastAsia_Font_File", DEFAULT_FONT_FILE.data());
    emojiFontFile         = ini.GetValue("General", "Emoji_Font_File", DEFAULT_EMOJI_FONT_FILE.data());
    fontSize              = static_cast<float>(ini.GetDoubleValue("General", "Font_Size", DEFAULT_FONT_SIZE));
    toolWindowShortcutKey = ini.GetLongValue("General", "Tool_Window_Shortcut_Key", DEFAULT_TOOL_WINDOW_SHORTCUT_KEY);
    auto alogLevel        = ini.GetLongValue("General", "Log_Level", DEFAULT_LOG_LEVEL);
    auto aflushLevel      = ini.GetLongValue("General", "Flush_Level", DEFAULT_FLUSH_LEVEL);
    logLevel              = parseLogLevel(alogLevel);
    flushLevel            = parseLogLevel(aflushLevel);
    auto textColor_       = ini.GetValue("General", "Text_Color", DEFAULT_TEXT_COL.data());
    auto highlightTextColor_ = ini.GetValue("General", "Highlight_Text_Color", DEFAULT_HIGHLIGHT_TEXT_COL.data());
    auto windowBorderColor_  = ini.GetValue("General", "Window_Border_Color", DEFAULT_WINDOW_BORDER_COL.data());
    auto windowBgColor_      = ini.GetValue("General", "Window_Bg_Color", DEFAULT_WINDOW_BG_COL.data());
    auto btnColor_           = ini.GetValue("General", "Button_color", DEFAULT_BUTTON_COL.data());

    textColor                = HexStringToUInt32(textColor_, 0xFFCCCCCC);
    highlightTextColor       = HexStringToUInt32(highlightTextColor_, 0xFFFFD700);
    windowBorderColor        = HexStringToUInt32(windowBorderColor_, 0xFF3A3A3A);
    windowBgColor            = HexStringToUInt32(windowBgColor_, 0x801E1E1E);
    btnColor                 = HexStringToUInt32(btnColor_, 0xFF444444);
}

uint32_t LIBC_NAMESPACE::SimpleIME::AppConfig::HexStringToUInt32(const std::string &hexStr, uint32_t aDefault)
{
    try
    {
        size_t start = (hexStr.find("0x") == 0 || hexStr.find("0X") == 0) ? 2 : 0;
        return static_cast<uint32_t>(std::stoul(hexStr.substr(start), nullptr, 16));
    }
    catch (const std::exception &e)
    {
        log_error("Error: {},not a hex string: {}", e.what(), hexStr.c_str());
    }
    return aDefault;
}

auto LIBC_NAMESPACE::SimpleIME::AppConfig::parseLogLevel(int32_t intLevel) -> spdlog::level::level_enum
{
    if (intLevel < 0 || intLevel >= static_cast<int>(spdlog::level::n_levels))
    {
        log_error("Invalid spdlog level {}, downgrade to default: info", intLevel);
        return spdlog::level::info;
    }
    return static_cast<spdlog::level::level_enum>(intLevel);
}