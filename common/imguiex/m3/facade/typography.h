//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "../spec/typography.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using LabelLarge     = TypeScale<TextRole::LabelLarge>;
using LabelMedium    = TypeScale<TextRole::LabelMedium>;
using LabelSmall     = TypeScale<TextRole::LabelSmall>;
using TitleSmall     = TypeScale<TextRole::TitleSmall>;
using TitleMedium    = TypeScale<TextRole::TitleMedium>;
using TitleLarge     = TypeScale<TextRole::TitleLarge>;
using HeadlineSmall  = TypeScale<TextRole::HeadlineSmall>;
using HeadlineMedium = TypeScale<TextRole::HeadlineMedium>;
using HeadlineLarge  = TypeScale<TextRole::HeadlineLarge>;
using BodySmall      = TypeScale<TextRole::BodySmall>;
using BodyMedium     = TypeScale<TextRole::BodyMedium>;
using BodyLarge      = TypeScale<TextRole::BodyLarge>;
using DisplaySmall   = TypeScale<TextRole::DisplaySmall>;
using DisplayMedium  = TypeScale<TextRole::DisplayMedium>;
using DisplayLarge   = TypeScale<TextRole::DisplayLarge>;
} // namespace ImGuiEx::M3::Spec
