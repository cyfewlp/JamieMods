//
// Created by jamie on 2026/2/8.
//
#pragma once
#include <cstdint>

namespace ImGuiEx::M3::Spec
{

struct Search
{
    static constexpr uint8_t paddingY = 4; // 16dp
    static constexpr uint8_t paddingX = 4; // 16dp
    static constexpr uint8_t rounding = 7; // 28dp
    static constexpr uint8_t gap      = 4; // 16dp
};

struct List
{
    static constexpr uint8_t paddingY = 4; // 16dp
    static constexpr uint8_t paddingX = 4; // 16dp
};

} // namespace ImGuiEx::M3::Spec
