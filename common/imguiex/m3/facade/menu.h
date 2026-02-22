//
// Created by jamie on 2026/2/19.
//

#pragma once

#include "../spec/menu.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using MenusVibrant    = Menus<MenuLayout::Standard, MenuVariant::Vibrant>;
using MenuItemVibrant = Menus<MenuLayout::Standard, MenuVariant::Vibrant>::Item;

using MenusHorizontalIconOnly    = Menus<MenuLayout::HorizontalIconOnly, MenuVariant::Standard>;
using MenuItemHorizontalIconOnly = Menus<MenuLayout::HorizontalIconOnly, MenuVariant::Standard>::Item;

template <>
struct Menus<MenuLayout::HorizontalIconOnly, MenuVariant::Vibrant> : public MenusVibrant, MenusHorizontalIconOnly
{
    using MenusHorizontalIconOnly::MenusHorizontalIconOnly;
    using MenusVibrant::MenusVibrant;

    struct Item : public MenuItemVibrant, MenuItemHorizontalIconOnly
    {
        using MenuItemHorizontalIconOnly::MenuItemHorizontalIconOnly;
        using MenuItemVibrant::MenuItemVibrant;
    };
};

using MenusHorizontalIconOnlyVibrant    = Menus<MenuLayout::HorizontalIconOnly, MenuVariant::Vibrant>;
using MenuItemHorizontalIconOnlyVibrant = Menus<MenuLayout::HorizontalIconOnly, MenuVariant::Vibrant>::Item;

using MenusHorizontal    = Menus<MenuLayout::Horizontal, MenuVariant::Standard>;
using MenuItemHorizontal = Menus<MenuLayout::Horizontal, MenuVariant::Standard>::Item;

template <>
struct Menus<MenuLayout::Horizontal, MenuVariant::Vibrant> : public MenusVibrant, MenuItemHorizontal
{
    using MenuItemHorizontal::MenuItemHorizontal;
    using MenusVibrant::MenusVibrant;

    struct Item : public MenuItemVibrant, MenuItemHorizontal
    {
        using MenuItemHorizontal::MenuItemHorizontal;
        using MenuItemVibrant::MenuItemVibrant;
    };
};

using MenusHorizontalVibrant    = Menus<MenuLayout::Horizontal, MenuVariant::Vibrant>;
using MenuItemHorizontalVibrant = Menus<MenuLayout::Horizontal, MenuVariant::Vibrant>::Item;

} // namespace ImGuiEx::M3::Spec
