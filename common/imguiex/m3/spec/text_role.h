//
// Created by jamie on 2026/2/10.
//

#pragma once

#include <cstdint>

namespace ImGuiEx::M3::Spec
{
/////////////////////////////////////////////////
/// ... Typography ...
/////////////////////////////////////////////////

enum class TextRole : std::uint8_t
{
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
struct Text
{
    static_assert(static_cast<int>(Role) < 0, "TextSpec not specialized for this role");
};

template <>
struct Text<TextRole::LabelSmall>
{
    static constexpr float fontSize   = 11.F;
    static constexpr float lineHeight = 16.F;
};

template <>
struct Text<TextRole::LabelMedium>
{
    static constexpr float fontSize   = 12.F;
    static constexpr float lineHeight = 16.F;
};

template <>
struct Text<TextRole::LabelLarge>
{
    static constexpr float fontSize   = 14.F;
    static constexpr float lineHeight = 20.F;
};
} // namespace ImGuiEx::M3::Spec
