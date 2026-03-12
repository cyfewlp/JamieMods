//
// Created by jamie on 2026/3/12.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{
struct Checkbox
{
    static constexpr auto LabelTextEx    = TypeScaleValue::of<TextRole::LabelLarge>();
    static constexpr auto LabelTextColor = ColorRole::onSurface;

    //! Checkbox icon size
    static constexpr auto LayoutSize = dp<48>();

    //! Checkbox icon size
    static constexpr auto IconSize = dp<18>();

    //! Checkbox container size
    static constexpr auto ContainerSize = dp<18>();

    //! Checkbox outline width
    static constexpr auto OutlineWidth = 2.0F;

    //! Checkbox focus indicator thickness
    static constexpr auto FocusIndicatorThickness = 3.0F;

    //! Checkbox focus indicator offset
    static constexpr auto FocusIndicatorOutlineOffset = dp<2>();

    //! Checkbox selected hover state layer color
    static constexpr auto SelectedStateLayerColor = ColorRole::onPrimary;
    static constexpr auto StateLayerColor         = ColorRole::onSurface;

    //! Checkbox unselected outline color
    static constexpr auto UnselectedOutlineColor = ColorRole::onSurfaceVariant;

    //! Checkbox unselected error outline color
    static constexpr auto UnselectedErrorOutlineColor = ColorRole::error;

    //! Checkbox selected icon color
    static constexpr auto SelectedIconColor = ColorRole::onPrimary;

    //! Checkbox selected error icon color
    static constexpr auto SelectedErrorIconColor = ColorRole::onError;

    //! Checkbox selected container color
    static constexpr auto SelectedContainerColor = ColorRole::primary;

    //! Checkbox selected error container color
    static constexpr auto SelectedErrorContainerColor = ColorRole::error;
    //! Checkbox focus indicator color
    static constexpr auto FocusIndicatorColor         = ColorRole::secondary;
};

} // namespace ImGuiEx::M3::Spec
