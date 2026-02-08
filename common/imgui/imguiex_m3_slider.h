//
// Created by jamie on 2026/2/8.
//

#pragma once

#include "Material3.h"
#include "imguiex_enum_wrap.h"

#include <cstdint>
#include <string_view>

namespace ImGuiEx::M3::Slider
{
template <SizeTips Size>
struct Spec;

template <>
struct Spec<SizeTips::XSMALL>
{
    static constexpr uint8_t grabOuterWidth = 4;  // 16dp
    static constexpr uint8_t grabHeight     = 11; // 44dp
    static constexpr uint8_t frameHeight    = 4;  // 16dp
};

template <>
struct Spec<SizeTips::SMALL>
{
    static constexpr uint8_t grabOuterWidth = 4;  // 16dp
    static constexpr uint8_t grabHeight     = 11; // 44dp
    static constexpr uint8_t frameHeight    = 6;  // 24dp
};

template <>
struct Spec<SizeTips::MEDIUM>
{
    static constexpr uint8_t grabOuterWidth = 4;  // 16dp
    static constexpr uint8_t grabHeight     = 13; // 44dp
    static constexpr uint8_t frameHeight    = 10; // 40dp
};

template <>
struct Spec<SizeTips::LARGE>
{
    static constexpr uint8_t grabOuterWidth = 4;  // 16dp
    static constexpr uint8_t grabHeight     = 17; // 68dp
    static constexpr uint8_t frameHeight    = 14; // 56dp
};

template <>
struct Spec<SizeTips::XLARGE>
{
    static constexpr uint8_t grabOuterWidth = 4;  // 16dp
    static constexpr uint8_t grabHeight     = 27; // 108dp
    static constexpr uint8_t frameHeight    = 24; // 96dp
};

namespace detail
{

template <class T>
constexpr auto ImGuiDataTypeFor() -> ImGuiDataType = delete;

// clang-format off

template <> constexpr auto ImGuiDataTypeFor<float>() -> ImGuiDataType { return ImGuiDataType_Float; }
template <> constexpr auto ImGuiDataTypeFor<double>() -> ImGuiDataType { return ImGuiDataType_Double; }
template <> constexpr auto ImGuiDataTypeFor<int>() -> ImGuiDataType { return ImGuiDataType_S32; }
template <> constexpr auto ImGuiDataTypeFor<uint32_t>() -> ImGuiDataType { return ImGuiDataType_U32; }
template <> constexpr auto ImGuiDataTypeFor<int64_t>() -> ImGuiDataType { return ImGuiDataType_S64; }
template <> constexpr auto ImGuiDataTypeFor<uint64_t>() -> ImGuiDataType { return ImGuiDataType_U64; }

// clang-format on

struct Params
{
    void           *pValue;
    const void     *pMinValue;
    const void     *pMaxValue;
    const M3Styles &m3Styles;
    double          value01;
    ImGuiDataType   dataType;
    uint8_t         grabOuterWidth;
    uint8_t         grabHeight;
    uint8_t         frameHeight;
};

auto Draw(std::string_view label, const Params &params, SliderFlags flags) -> bool;
} // namespace detail

template <typename T>
struct Params
{
    T              &value;
    T               minValue;
    T               maxValue;
    const M3Styles &m3Styles;
};

template <SizeTips Size = SizeTips::SMALL, typename T>
auto Draw(const std::string_view label, const Params<T> &params, const SliderFlags flags = {}) -> bool
{
    const ImGuiDataType dataType = detail::ImGuiDataTypeFor<T>();

    auto minValue = params.minValue;
    auto maxValue = params.maxValue;
    if (minValue > maxValue) std::swap(minValue, maxValue);
    const double value01 = (maxValue != minValue)
                               ? (static_cast<double>(params.value) - static_cast<double>(minValue)) /
                                     (static_cast<double>(maxValue) - static_cast<double>(minValue))
                               : 0.0;
    return detail::Draw(
        label,
        detail::Params{
            .pValue         = &params.value,
            .pMinValue      = &minValue,
            .pMaxValue      = &maxValue,
            .m3Styles       = params.m3Styles,
            .value01        = value01,
            .dataType       = dataType,
            .grabOuterWidth = Spec<Size>::grabOuterWidth,
            .grabHeight     = Spec<Size>::grabHeight,
            .frameHeight    = Spec<Size>::frameHeight,
        },
        flags
    );
}

} // namespace ImGuiEx::M3::Slider
