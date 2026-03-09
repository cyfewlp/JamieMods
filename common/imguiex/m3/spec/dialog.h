//
// Created by jamie on 2026/3/9.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{

struct Dialog
{
    static constexpr Unit  PaddingX           = dp<24>();
    static constexpr Unit  PaddingY           = dp<24>();
    static constexpr Unit  MinWidth           = dp<280>();
    static constexpr float MaxWidth           = 560.0F;
    static constexpr Unit  TitleBodyPadding   = dp<16>(); ///< Padding between title and body
    static constexpr Unit  IconTiltePadding   = dp<16>(); ///< Padding between icon and title
    static constexpr Unit  BodyActionsPadding = dp<24>(); ///< Padding between body and actions
    static constexpr Unit  ButtonsPadding     = dp<8>();  ///< Padding between buttons
    //! Dialog icon size
    static constexpr Unit  WithIconIconSize   = dp<24>();
    //! Dialog container shape
    static constexpr Unit  ContainerShape     = ShapeCorner::ExtraLarge;

    //! Dialog headline size
    static constexpr auto HeadlineText = TypeScaleValue::of<TextRole::HeadlineSmall>();

    //! Dialog supporting text size
    static constexpr auto SupportingText = TypeScaleValue::of<TextRole::BodyMedium>();

    //! Dialog action label text size
    static constexpr auto ActionLabelText = TypeScaleValue::of<TextRole::LabelLarge>();

    //! Dialog headline color
    static constexpr auto HeadlineColor = ColorRole::onSurface;

    //! Dialog icon color
    static constexpr auto WithIconIconColor = ColorRole::secondary;

    //! Dialog container color
    static constexpr auto ContainerColor = ColorRole::surfaceContainerHigh;

    //! Dialog supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurfaceVariant;

    //! Dialog action label text color
    static constexpr auto ActionLabelTextColor = ColorRole::primary;
};

struct FullScreenDialog
{
    //! Full screen dialog container shape
    static constexpr auto ContainerShape = ShapeCorner::None;

    //! Full screen dialog header headline size
    static constexpr auto HeaderHeadlineText = TypeScaleValue::of<TextRole::TitleLarge>();

    //! Full screen dialog header icon size
    static constexpr auto HeaderIconSize = dp<24>();

    //! Full screen dialog header action label text size
    static constexpr auto HeaderActionLabelText = TypeScaleValue::of<TextRole::LabelLarge>();

    //! Full screen dialog header headline color
    static constexpr auto HeaderHeadlineColor = ColorRole::onSurface;

    //! Full screen dialog header icon color
    static constexpr auto HeaderIconColor = ColorRole::onSurface;

    //! Full screen dialog container color
    static constexpr auto ContainerColor = ColorRole::surface;

    //! Full screen dialog on-scroll container color
    static constexpr auto HeaderOnScrollContainerColor = ColorRole::surfaceContainer;

    //! Full screen dialog header container color
    static constexpr auto HeaderContainerColor = ColorRole::surface;

    //! Full screen dialog header action label text color
    static constexpr auto HeaderActionLabelTextColor = ColorRole::primary;

    //! Full screen dialog header container height
    static constexpr auto HeaderContainerHeight = dp<56>();
};

} // namespace ImGuiEx::M3::Spec
