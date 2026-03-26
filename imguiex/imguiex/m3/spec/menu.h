//
// Created by jamie on 2026/2/19.
//
#pragma once
#include "../colors.h"
#include "base.h"
#include "shapes.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{
struct Menu
{
    //! Menu container elevation
    // static constexpr auto ContainerElevation = elevation.level2;
    //! Menu container shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;
    //! Menu container shape
    static constexpr auto ContainerShape       = ShapeCorner::Large;
};

enum class MenuColors : std::uint8_t
{
    Standard,
    Vibrant
};

enum class MenuLayout : std::uint8_t
{
    Standard,
    Horizontal,
    HorizontalIconOnly
};

template <MenuColors Colors>
struct Menus;

template <>
struct Menus<MenuColors::Standard>
{
    //! Menu standard section label text color
    static constexpr auto SectionLabelTextColor            = ColorRole::onSurfaceVariant;
    //! Menu standard icon button disabled icon color
    static constexpr auto IconButtonDisabledIconColor      = ColorRole::onTertiaryContainer;
    //! Menu standard icon button selected icon color
    static constexpr auto IconButtonSelectedIconColor      = ColorRole::onTertiaryContainer;
    //! Menu standard icon button selected container color
    static constexpr auto IconButtonSelectedContainerColor = ColorRole::tertiaryContainer;
    //! Menu standard icon button focused state layer color
    static constexpr auto IconButtonFocusedStateLayerColor = ColorRole::onSurface;
    //! Menu standard icon button pressed state layer color
    static constexpr auto IconButtonPressedStateLayerColor = ColorRole::onSurface;
    //! Menu standard icon button hovered state layer color
    static constexpr auto IconButtonHoveredStateLayerColor = ColorRole::onSurface;
    //! Menu standard disabled icon button icon color
    static constexpr auto DisabledIconButtonIconColor      = ColorRole::onSurface;
    //! Menu standard icon button icon color
    static constexpr auto IconButtonIconColor              = ColorRole::onSurfaceVariant;
    //! Menu standard icon button container color
    static constexpr auto IconButtonContainerColor         = ColorRole::surfaceContainerLow;
    //! Menu standard container shadow color
    static constexpr auto ContainerShadowColor             = ColorRole::shadow;
    //! Menu standard container color
    static constexpr auto ContainerColor                   = ColorRole::surfaceContainerLow;

    struct Item
    {
        //! Menu item focus indicator color
        static constexpr auto FocusIndicatorColor                         = ColorRole::secondary;
        //! Menu standard menu item container color
        static constexpr auto ContainerColor                              = ColorRole::surfaceContainerLow;
        //! Menu standard menu item leading icon color (applies to focused, pressed, hovered, and default)
        static constexpr auto LeadingIconColor                            = ColorRole::onSurfaceVariant;
        //! Menu standard menu item disabled leading icon color
        static constexpr auto DisabledLeadingIconColor                    = ColorRole::onSurface;
        //! Menu standard menu item trailing icon color (applies to focused, pressed, hovered, and default)
        static constexpr auto TrailingIconColor                           = ColorRole::onSurfaceVariant;
        //! Menu standard menu item disabled trailing icon color
        static constexpr auto DisabledTrailingIconColor                   = ColorRole::onSurface;
        //! Menu standard menu item supporting text color (applies to focused, pressed, hovered, and default)
        static constexpr auto SupportingTextColor                         = ColorRole::onSurfaceVariant;
        //! Menu standard menu item disabled supporting text color
        static constexpr auto DisabledSupportingTextColor                 = ColorRole::onSurface;
        //! Menu standard menu item trailing supporting text color (applies to focused, pressed, hovered, and default)
        static constexpr auto TrailingSupportingTextColor                 = ColorRole::onSurfaceVariant;
        //! Menu standard menu item disabled trailing supporting text color
        static constexpr auto DisabledTrailingSupportingTextColor         = ColorRole::onSurface;
        //! Menu standard menu item label text color (applies to focused, pressed, hovered, and default)
        static constexpr auto LabelTextColor                              = ColorRole::onSurface;
        //! Menu standard menu item disabled label text color
        static constexpr auto DisabledLabelTextColor                      = ColorRole::onSurface;
        // Selected state (separate set with different values)
        //! Menu standard menu item selected container color
        static constexpr auto SelectedContainerColor                      = ColorRole::tertiaryContainer;
        //! Menu standard menu item selected label text color
        static constexpr auto SelectedLabelTextColor                      = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected supporting text color
        static constexpr auto SelectedSupportingTextColor                 = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected leading icon color
        static constexpr auto SelectedLeadingIconColor                    = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected trailing icon color
        static constexpr auto SelectedTrailingIconColor                   = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected trailing supporting text color
        static constexpr auto SelectedTrailingSupportingTextColor         = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected disabled container color
        static constexpr auto SelectedDisabledContainerColor              = ColorRole::tertiaryContainer;
        static constexpr auto SelectedDisabledContainerOpacity            = 0.38F;
        //! Menu standard menu item selected disabled label text color
        static constexpr auto SelectedDisabledLabelTextColor              = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected disabled leading icon color
        static constexpr auto SelectedDisabledLeadingIconColor            = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected disabled trailing icon color
        static constexpr auto SelectedDisabledTrailingIconColor           = ColorRole::onTertiaryContainer;
        //! Menu standard menu item selected disabled trailing supporting text color
        static constexpr auto SelectedDisabledTrailingSupportingTextColor = ColorRole::onTertiaryContainer;
    };
};

template <>
struct Menus<MenuColors::Vibrant>
{
    //! Menu vibrant section label text color
    static constexpr auto SectionLabelTextColor               = ColorRole::onTertiaryContainer;
    //! Menu vibrant container color
    static constexpr auto ContainerColor                      = ColorRole::tertiaryContainer;
    //! Menu vibrant icon button icon color
    static constexpr auto IconButtonIconColor                 = ColorRole::onTertiaryContainer;
    //! Menu vibrant icon button container color
    static constexpr auto IconButtonContainerColor            = ColorRole::tertiaryContainer;
    //! Menu vibrant icon button selected disabled icon color
    static constexpr auto IconButtonSelectedDisabledIconColor = ColorRole::onTertiary;
    //! Menu vibrant icon button selected icon color
    static constexpr auto IconButtonSelectedIconColor         = ColorRole::onTertiary;
    //! Menu vibrant icon button selected container color
    static constexpr auto IconButtonSelectedContainerColor    = ColorRole::tertiary;
    //! Menu vibrant icon button disabled icon color
    static constexpr auto IconButtonDisabledIconColor         = ColorRole::onTertiaryContainer;

    struct Item
    {
        // Non-selected state
        //! Menu vibrant menu item label text color
        static constexpr auto LabelTextColor              = ColorRole::onTertiaryContainer;
        //! Menu vibrant menu item supporting text color
        static constexpr auto SupportingTextColor         = ColorRole::onTertiaryContainer;
        //! Menu vibrant menu item trailing supporting text color
        static constexpr auto TrailingSupportingTextColor = ColorRole::onTertiaryContainer;
        //! Menu vibrant menu item leading icon color
        static constexpr auto LeadingIconColor            = ColorRole::onTertiaryContainer;
        //! Menu vibrant menu item trailing icon color
        static constexpr auto TrailingIconColor           = ColorRole::onTertiaryContainer;
        //! Menu vibrant menu item container color
        static constexpr auto ContainerColor              = ColorRole::tertiaryContainer;

        // Selected state (separate set with different values)
        //! Menu vibrant menu item selected container color
        static constexpr auto SelectedContainerColor              = ColorRole::tertiary;
        //! Menu vibrant menu item selected label text color
        static constexpr auto SelectedLabelTextColor              = ColorRole::onTertiary;
        //! Menu vibrant menu item selected supporting text color
        static constexpr auto SelectedSupportingTextColor         = ColorRole::onTertiary;
        //! Menu vibrant menu item selected trailing supporting text color
        static constexpr auto SelectedTrailingSupportingTextColor = ColorRole::onTertiary;
        //! Menu vibrant menu item selected leading icon color
        static constexpr auto SelectedLeadingIconColor            = ColorRole::onTertiary;
        //! Menu vibrant menu item selected trailing icon color
        static constexpr auto SelectedTrailingIconColor           = ColorRole::onTertiary;
    };
};

template <MenuLayout Layout>
struct MenusSizing;

template <>
struct MenusSizing<MenuLayout::Standard>
{
    //! Menu container elevation
    // static constexpr auto ContainerElevation = elevation.level2;
    //! Menu gap
    static constexpr auto Gap                       = dp<2>();
    //! Extent field. Top gap + bottom gap.
    static constexpr auto TotalGapEx                = Gap * 2U;
    //! Menu group padding
    static constexpr auto GroupPadding              = dp<2>();
    //! Menu inactive container shape
    static constexpr auto InactiveContainerShape    = ShapeCorner::Small;
    //! Menu group shape
    static constexpr auto GroupShape                = ShapeCorner::Small;
    //! Menu container shape
    static constexpr auto ShapeCornerContainerShape = ShapeCorner::Large;
    //! Menu active container shape
    static constexpr auto ActiveContainerShape      = ShapeCorner::Large;

    struct Item
    {
        //! Menu item height
        static constexpr auto  Height                              = dp<44>();
        //! Menu item trailing space
        static constexpr auto  TrailingSpace                       = dp<16>();
        //! Menu item leading space
        static constexpr auto  LeadingSpace                        = dp<16>();
        //! Menu item trailing icon size
        static constexpr auto  TrailingIconSize                    = dp<20>();
        //! Menu item top space
        static constexpr auto  TopSpace                            = dp<8>();
        //! Menu item between space
        static constexpr auto  BetweenSpace                        = dp<12>();
        //! Menu item bottom space
        static constexpr auto  BottomSpace                         = dp<8>();
        //! Menu item leading icon size
        static constexpr auto  LeadingIconSize                     = dp<20>();
        //! Menu item outer height. The total height of a menu item, including the inherent height and the top and
        //! bottom gaps.
        static constexpr auto  OuterHeightEx                       = Height + TotalGapEx;
        //! M3 spec defined BetweenSpaceEx length inconsistent with that length defined in Figma.
        //! We follow the Figma design in this case, and use a different name to avoid confusion.
        static constexpr auto  BetweenSpaceEx                      = dp<8>();
        //! Extent field. The menu item horizontal offset from the menu eage.
        static constexpr auto  OffsetXEx                           = dp<4>();
        //! The inherent fixed width of a text-only menu item (excluding label text).
        //! Calculation: LeadingSpace + TrailingSpace + (OffsetXEx * 2)
        static constexpr auto  MinWidthEx                          = LeadingSpace + TrailingSpace + (OffsetXEx * 2U);
        //! The inherent fixed width of a menu item with a leading icon (excluding label text).
        //! Calculation: BaseWidth + LeadingIconSize + BetweenSpace
        static constexpr auto  MinWidthWithIconEx                  = MinWidthEx + LeadingIconSize + BetweenSpace;
        //! The inherent fixed width of a menu item with both leading and trailing icons (excluding label text).
        //! Calculation: BaseWidthWithIcon + BetweenSpace + TrailingIconSize
        static constexpr auto  MinWidthWithIconsEx                 = MinWidthWithIconEx + BetweenSpace + TrailingIconSize;
        //! Menu item label text size
        static constexpr auto  LabelTextRole                       = TextRole::LabelLarge;
        //! Menu item trailing supporting text size
        static constexpr auto  TrailingSupportingTextRole          = TextRole::LabelLarge;
        //! Menu item supporting text size
        static constexpr auto  SupportingTextRole                  = TextRole::BodySmall;
        static constexpr auto  TopSpaceEx                          = dp<4>();
        //! Menu item focus indicator outline offset
        static constexpr float MenuItemFocusIndicatorOutlineOffset = -3.0F;
        //! Menu item focus indicator thickness
        static constexpr float MenuItemFocusIndicatorThickness     = 3.0F;
        //! Menu item last child inner corner corner size
        static constexpr auto  LastChildInnerCornerCornerSize      = ShapeCorner::ExtraSmall;
        //! Menu item first child shape
        static constexpr auto  FirstChildShape                     = ShapeCorner::Medium;
        //! Menu item first child inner corner corner size
        static constexpr auto  FirstChildInnerCornerCornerSize     = ShapeCorner::ExtraSmall;
        //! Menu item last child shape
        static constexpr auto  LastChildShape                      = ShapeCorner::Medium;
        //! Menu item shape
        static constexpr auto  Shape                               = ShapeCorner::ExtraSmall;
        //! Menu item selected shape
        static constexpr auto  SelectedShape                       = ShapeCorner::Medium;
    };
};

using MenusSizingStandard    = MenusSizing<MenuLayout::Standard>;
using MenuItemSizingStandard = MenusSizing<MenuLayout::Standard>::Item;

template <>
struct MenusSizing<MenuLayout::Horizontal> : MenusSizingStandard
{
    //! Menu horizontal container bottom space
    static constexpr auto ContainerBottomSpace = dp<8>();
    //! Menu horizontal container top space
    static constexpr auto ContainerTopSpace    = dp<8>();

    //! Menu horizontal container shape
    // static constexpr auto ContainerShape = ; ///< Full shape

    struct Item : public MenuItemSizingStandard
    {
        //! Menu horizontal menu item leading space
        static constexpr auto LeadingSpace         = dp<12>();
        //! Menu horizontal menu item bottom space
        static constexpr auto BottomSpace          = dp<6>();
        //! Menu horizontal menu item top space
        static constexpr auto TopSpace             = dp<6>();
        //! Menu horizontal menu item between space
        static constexpr auto BetweenSpace         = dp<12>();
        //! Menu horizontal menu item trailing space
        static constexpr auto TrailingSpace        = dp<12>();
        //! The inherent fixed width of a text-only menu item (excluding label text).
        //! Calculation: LeadingSpace + TrailingSpace + (OffsetXEx * 2)
        static constexpr auto MinWidthEx           = LeadingSpace + TrailingSpace + (OffsetXEx * 2U);
        //! Menu horizontal menu item focused shape
        static constexpr auto FocusedShape         = ShapeCorner::Medium;
        //! Menu horizontal menu item hovered shape
        static constexpr auto HoveredShape         = ShapeCorner::Medium;
        //! Menu horizontal menu item pressed shape
        static constexpr auto PressedShape         = ShapeCorner::Medium;
        //! Menu horizontal menu item selected pressed shape
        static constexpr auto SelectedPressedShape = Height / 2U; ///< ShapeCorner::full;
        //! Menu horizontal menu item selected focused shape
        static constexpr auto SelectedFocusedShape = Height / 2U; ///< ShapeCorner::full;
        //! Menu horizontal menu item selected hovered shape
        static constexpr auto SelectedHoveredShape = Height / 2U; ///< ShapeCorner::full;
    };
};

using MenusHorizontal    = MenusSizing<MenuLayout::Horizontal>;
using MenuItemHorizontal = MenusSizing<MenuLayout::Horizontal>::Item;

template <>
struct MenusSizing<MenuLayout::HorizontalIconOnly> : public MenusHorizontal
{
    using MenusHorizontal::MenusHorizontal;
    //! Menu horizontal icon only gap
    static constexpr auto Gap = dp<4>();

    struct Item : public MenuItemHorizontal
    {
        using MenuItemHorizontal::MenuItemHorizontal;

        //! Menu horizontal icon only menu item bottom space
        static constexpr auto BottomSpace           = dp<16>();
        //! Menu horizontal icon only menu item top space
        static constexpr auto TopSpace              = dp<16>();
        //! Menu horizontal icon only menu item trailing space
        static constexpr auto TrailingSpace         = dp<16>();
        //! Menu horizontal icon only menu item leading space
        static constexpr auto LeadingSpace          = dp<16>();
        //! Menu horizontal icon only menu item selected shape
        static constexpr auto MenuItemSelectedShape = Height / 2U; ///< ShapeCorner::full;
    };
};

struct MenuItemColorsValues
{
    ColorRole containerColor;
    ColorRole iconColor;
    float     selectedContainerOpacity;
    ColorRole labelTextColor;
    ColorRole supportingTextColor;
};

//! Get menu item colors values based on the menu colors and disabled state
//! @note The disabled state is currently not used for menu item colors, as the M3 spec does not define different colors for disabled menu items.
//! However, the parameter is included for future use in case the spec is updated to include disabled state colors.
constexpr auto GetMenuItemColors(MenuColors colors, const bool selected, [[maybe_unused]] const bool disabled) -> MenuItemColorsValues
{
    MenuItemColorsValues values{};
    switch (colors)
    {
        case MenuColors::Standard: {
            using ItemSpec                  = Menus<MenuColors::Standard>::Item;
            values.containerColor           = selected ? ItemSpec::SelectedContainerColor : ItemSpec::ContainerColor;
            values.iconColor                = selected ? ItemSpec::SelectedLeadingIconColor : ItemSpec::LeadingIconColor;
            values.labelTextColor           = selected ? ItemSpec::SelectedLabelTextColor : ItemSpec::LabelTextColor;
            values.supportingTextColor      = selected ? ItemSpec::SelectedSupportingTextColor : ItemSpec::SupportingTextColor;
            values.selectedContainerOpacity = disabled ? ItemSpec::SelectedDisabledContainerOpacity : 1.0F;
            break;
        }
        case MenuColors::Vibrant: {
            using ItemSpec                  = Menus<MenuColors::Vibrant>::Item;
            values.containerColor           = selected ? ItemSpec::SelectedContainerColor : ItemSpec::ContainerColor;
            values.iconColor                = selected ? ItemSpec::SelectedLeadingIconColor : ItemSpec::LeadingIconColor;
            values.labelTextColor           = selected ? ItemSpec::SelectedLabelTextColor : ItemSpec::LabelTextColor;
            values.supportingTextColor      = selected ? ItemSpec::SelectedSupportingTextColor : ItemSpec::SupportingTextColor;
            values.selectedContainerOpacity = 1.0F;
            break;
        }
    }
    return values;
}

} // namespace ImGuiEx::M3::Spec
