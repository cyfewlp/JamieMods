//
// Created by jamie on 2026/2/26.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{

struct SearchView
{
    static constexpr auto IconSize             = dp<24>();
    //! Search view docked container shape
    static constexpr auto DockedContainerShape = ShapeCorner::ExtraLarge;

    //! Search view full screen container shape
    static constexpr auto FullScreenContainerShape = ShapeCorner::None;

    //! Search view header supporting text size
    static constexpr auto HeaderSupportingText = TypeScaleValue::of<TextRole::BodyLarge>();

    //! Search view header input text size
    static constexpr auto HeaderInputText = TypeScaleValue::of<TextRole::BodyLarge>();

    //! Search view header leading icon color
    static constexpr auto HeaderLeadingIconColor = ColorRole::onSurface;

    //! Search view header trailing icon color
    static constexpr auto HeaderTrailingIconColor = ColorRole::onSurfaceVariant;

    //! Search view divider color
    static constexpr auto DividerColor = ColorRole::outline;

    //! Search view container color
    static constexpr auto ContainerColor = ColorRole::surfaceContainerHigh;

    //! Search view header supporting text color
    static constexpr auto HeaderSupportingTextColor = ColorRole::onSurfaceVariant;

    //! Search view header input text color
    static constexpr auto HeaderInputTextColor = ColorRole::onSurface;

    //! Search view docked header container height
    static constexpr auto DockedHeaderContainerHeight = dp<56>();

    //! Search view full screen header container height
    static constexpr auto FullScreenHeaderContainerHeight = dp<72>();
};

struct SearchBar
{
    static constexpr auto IconSize   = dp<24>();
    //! Search bar avatar size
    static constexpr auto AvatarSize = dp<30>();

    //! Search bar supporting text size
    static constexpr auto SupportingText = TypeScaleValue::of<TextRole::BodyLarge>();

    //! Search bar input text size
    static constexpr auto InputText = TypeScaleValue::of<TextRole::BodyLarge>();

    //! Search bar leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurface;

    //! Search bar trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! Search bar container color
    static constexpr auto ContainerColor = ColorRole::surfaceContainerHigh;

    //! Search bar focus indicator color
    static constexpr auto FocusIndicatorColor = ColorRole::secondary;

    //! Search bar supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurfaceVariant;

    //! Search bar input text color
    static constexpr auto InputTextColor = ColorRole::onSurface;

    //! Search bar focus indicator thickness
    static constexpr auto FocusIndicatorThickness = 3.0F;

    //! Search bar focus indicator offset
    static constexpr auto FocusIndicatorOutlineOffset = dp<2>();

    //! Search bar container height
    static constexpr auto ContainerHeight        = dp<56>();
    //! Search bar container min width
    static constexpr auto ContainerMinWidth      = 360.0F;
    //! Search bar container max width
    static constexpr auto ContainerMaxWidth      = 720.0F;
    static constexpr auto LeadingIconLabelSpace  = dp<16>();
    static constexpr auto TrailingIconLabelSpace = dp<16>();
    static constexpr auto LeadingSpace           = dp<16>();
    static constexpr auto TrailingSpace          = dp<16>();

    //! Search bar avatar shape
    static constexpr auto AvatarShape = ContainerHeight / 2U; ///< ShapeCorner::Full;

    //! Search bar container shape
    static constexpr auto ContainerShape = ContainerHeight / 2U; ///< ShapeCorner::Full;
};

} // namespace ImGuiEx::M3::Spec
