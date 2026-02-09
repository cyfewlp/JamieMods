//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "../spec/text_role.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using LabelLarge     = Text<TextRole::LabelLarge>;
using LabelMedium    = Text<TextRole::LabelMedium>;
using LabelSmall     = Text<TextRole::LabelSmall>;
using TitleSmall     = Text<TextRole::TitleSmall>;
using TitleMedium    = Text<TextRole::TitleMedium>;
using TitleLarge     = Text<TextRole::TitleLarge>;
using HeadlineSmall  = Text<TextRole::HeadlineSmall>;
using HeadlineMedium = Text<TextRole::HeadlineMedium>;
using HeadlineLarge  = Text<TextRole::HeadlineLarge>;
using BodySmall      = Text<TextRole::BodySmall>;
using BodyMedium     = Text<TextRole::BodyMedium>;
using BodyLarge      = Text<TextRole::BodyLarge>;
using DisplaySmall   = Text<TextRole::DisplaySmall>;
using DisplayMedium  = Text<TextRole::DisplayMedium>;
using DisplayLarge   = Text<TextRole::DisplayLarge>;
} // namespace ImGuiEx::M3::Spec
