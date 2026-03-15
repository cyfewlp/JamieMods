//
// Created by jamie on 2026/2/10.
//

#pragma once

#include <cstdint>

/**
 * [Typography](https://m3.material.io/styles/typography/type-scale-tokens)
 */
namespace ImGuiEx::M3::Spec
{
enum class TextRole : int
{
    None,
    LabelSmall,
    LabelMedium,
    LabelLarge,
    TitleSmall,
    TitleMedium,
    TitleLarge,
    HeadlineSmall,
    HeadlineMedium,
    HeadlineLarge,
    BodySmall,
    BodyMedium,
    BodyLarge,
    DisplaySmall,
    DisplayMedium,
    DisplayLarge
};

template <TextRole Role>
struct TypeScale
{
    static_assert(static_cast<int>(Role) < 0, "TextSpec not specialized for this role");
};

template <>
struct TypeScale<TextRole::LabelSmall>
{
    static constexpr float textSize   = 11.F;
    static constexpr float lineHeight = 16.F;
};

template <>
struct TypeScale<TextRole::LabelMedium>
{
    static constexpr float textSize   = 12.F;
    static constexpr float lineHeight = 16.F;
};

template <>
struct TypeScale<TextRole::LabelLarge>
{
    static constexpr float textSize   = 14.F;
    static constexpr float lineHeight = 20.F;
};

template <>
struct TypeScale<TextRole::BodySmall>
{
    static constexpr float textSize   = 12.F;
    static constexpr float lineHeight = 16.F;
};

template <>
struct TypeScale<TextRole::BodyMedium>
{
    static constexpr float textSize   = 14.F;
    static constexpr float lineHeight = 20.F;
};

template <>
struct TypeScale<TextRole::BodyLarge>
{
    static constexpr float textSize   = 16.F;
    static constexpr float lineHeight = 24.F;
};

template <>
struct TypeScale<TextRole::TitleSmall>
{
    static constexpr float textSize   = 14.F;
    static constexpr float lineHeight = 20.F;
};

template <>
struct TypeScale<TextRole::TitleMedium>
{
    static constexpr float textSize   = 16.F;
    static constexpr float lineHeight = 24.F;
};

template <>
struct TypeScale<TextRole::TitleLarge>
{
    static constexpr float textSize   = 22.F;
    static constexpr float lineHeight = 28.F;
};

template <>
struct TypeScale<TextRole::HeadlineSmall>
{
    static constexpr float textSize   = 24.F;
    static constexpr float lineHeight = 32.F;
};

template <>
struct TypeScale<TextRole::HeadlineMedium>
{
    static constexpr float textSize   = 28.F;
    static constexpr float lineHeight = 36.F;
};

template <>
struct TypeScale<TextRole::HeadlineLarge>
{
    static constexpr float textSize   = 32.F;
    static constexpr float lineHeight = 40.F;
};

template <>
struct TypeScale<TextRole::DisplaySmall>
{
    static constexpr float textSize   = 36.F;
    static constexpr float lineHeight = 44.F;
};

template <>
struct TypeScale<TextRole::DisplayMedium>
{
    static constexpr float textSize   = 45.F;
    static constexpr float lineHeight = 52.F;
};

template <>
struct TypeScale<TextRole::DisplayLarge>
{
    static constexpr float textSize   = 57.F;
    static constexpr float lineHeight = 64.F;
};

struct TypeScaleValue
{
    TextRole role;
    float    textSize;
    float    lineHeight;

    template <TextRole Role>
    static constexpr auto of() -> TypeScaleValue
    {
        return {.role = Role, .textSize = TypeScale<Role>::textSize, .lineHeight = TypeScale<Role>::lineHeight};
    }
};

} // namespace ImGuiEx::M3::Spec
