//
// Created by jamie on 2026/1/18.
//

#pragma once

#include "common/config.h"
#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
/// https://m3.material.io/
namespace Material3Styles
{

static constexpr auto CUSTOM_WINDOW_PADDING1       = ImVec2{32.f, 32.f};
static constexpr auto CUSTOM_WINDOW_PADDING2       = ImVec2{16.f, 16.f};
static constexpr auto CUSTOM_WINDOW_PADDING4       = ImVec2{8.f, 8.f};
static constexpr auto CUSTOM_THICK_SCROLL_BAR_SIZE = 8.f;

struct ButtonStyle
{
    float  fontSize;
    ImVec2 padding;
    ImVec2 spacing;
    float  rounding;
};

static constexpr auto XSMALL_BUTTON = ButtonStyle{
    .fontSize = 20.f, .padding = ImVec2{12.f, 6.f },
         .spacing = ImVec2{18.f, 16.f},
         .rounding = 12.f
};

static constexpr auto SMALL_BUTTON = ButtonStyle{
    .fontSize = 20.f, .padding = ImVec2{16.f, 10.f},
         .spacing = ImVec2{12.f, 8.f },
         .rounding = 12.f
};

static constexpr auto XSMALL_ICON_BUTTON = ButtonStyle{
    .fontSize = 20.f, .padding = ImVec2{6.f,  6.f },
         .spacing = ImVec2{16.f, 16.f},
         .rounding = 8.f
};

static constexpr auto SMALL_ICON_BUTTON = ButtonStyle{
    .fontSize = 20.f, .padding = ImVec2{10.f, 10.f},
         .spacing = ImVec2{8.f,  8.f },
         .rounding = 8.f
};

struct ListStyle
{
    float  fontSize;
    float  supportFontSize;
    ImVec2 padding;
};

static constexpr auto LIST_2DENSITY = ListStyle{
    .fontSize        = 24.f,
    .supportFontSize = 20.f,
    .padding         = ImVec2{8.f, 12.f},
};

static constexpr auto LIST_4DENSITY = ListStyle{
    .fontSize        = 24.f,
    .supportFontSize = 20.f,
    .padding         = ImVec2{8.f, 8.f},
};

struct ToolBarStyle
{
    ImVec2 padding;
    float  rounding;
};

static constexpr auto TOOL_BAR_STANDARD = ToolBarStyle{
    .padding = ImVec2{8.f, 8.f},
      .rounding = 24.f
};

}
}