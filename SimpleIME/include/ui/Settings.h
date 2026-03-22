//
// Created by jamie on 2025/5/21.
//
#pragma once

#include "imgui.h"
#include "imguiex/m3/colors.h"

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
    static constexpr std::wstring_view DEFAULT_EMOJI_FONT_FAMILY  = L"Segoe UI Emoji";
    static constexpr std::wstring_view DEFAULT_SYMBOL_FONT_FAMILY = L"Segoe UI Symbol";
    static constexpr std::string_view  ICON_FILE                  = "simple-ime-icons.ttf";
    static constexpr auto              ZOOM_MAX                   = 2.0F;
    static constexpr auto              ZOOM_MIN                   = 0.5F;
    static constexpr int               ZOOM_STEP_PERCENT          = 25;

    //! UI layer is immediate mode(ImGui), no event callback, no commpoent search. So we need a way to present the UI layer state.
    //! This struct is for that.
    struct RuntimeData
    {
        bool requestCloseTopWindow = false; ///< Request to close the current top-level UI window on the next frame.

        bool overlayPinned     = false; ///< Overlay has been pinned by the user and should not auto-hide automatically.
        bool overlayShowing    = false; ///< Overlay is currently visible/rendered in this frame.
        bool toolWindowShowing = false; ///< Auxiliary tool/settings window is currently visible.
    } runtimeData;

    //! Shortcut: support combination of Ctrl, Shift, Alt and a normal key. e.g. "ctrl+shift+f1", "alt+f2", "f3"...
    //! The named key is can't combine by bitwise operation, g.g. "F2 | A" will become to "F3".
    ImGuiKeyChord shortcut;
    bool          enableMod                     = true; ///< modify on UI thread every frame.
    bool          enableTsf                     = true;
    bool          fixInconsistentTextEntryCount = true; ///< modify in ToolWindow(ImeMenu). no need sync because ImeMenu is the topmost menu;

    struct Logging
    {
        spdlog::level::level_enum level;
        spdlog::level::level_enum flushLevel;
    } logging;

    struct Resources
    {
        std::string              translationDir;
        std::vector<std::string> fontPathList;
    } resources;

    struct Appearance
    {
        ImGuiEx::M3::SchemeConfig schemeConfig; ///< modify in runtime by AppearancePanel; read once on Mod launch by ImeApp;
        std::string               language;
        float                     zoom; ///< modify in runtime by AppearancePanel; read once on Mod launch by ImeApp;
        int                       errorDisplayDuration;
        bool                      verticalCandidateList; ///< modify/read in runtime by UI thread.
    } appearance;

    struct Input
    {
        bool                  enableUnicodePaste;
        bool                  keepImeOpen;
        WindowPosUpdatePolicy posUpdatePolicy;
    } input;
};

inline auto GetDefaultSettings() -> Settings
{
    return {
        .runtimeData                   = {},
        .shortcut                      = ImGuiKey_F2,
        .enableMod                     = true,
        .enableTsf                     = true,
        .fixInconsistentTextEntryCount = true,
        .logging                       = {.level = spdlog::level::info, .flushLevel = spdlog::level::info},
        .resources                     = {.translationDir = "Data/interface/SimpleIME", .fontPathList = {}},
        .appearance =
            {.schemeConfig          = ImGuiEx::M3::GetM3ClassicSchemeConfig(),
                                          .language              = "english",
                                          .zoom                  = -1.0F,
                                          .errorDisplayDuration  = 10,
                                          .verticalCandidateList = false},
        .input = {.enableUnicodePaste = true, .keepImeOpen = false, .posUpdatePolicy = Settings::WindowPosUpdatePolicy::BASED_ON_CARET}
    };
}

} // namespace Ime
