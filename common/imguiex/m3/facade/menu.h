//
// Created by jamie on 2026/2/19.
//

#pragma once

#include "../spec/menu.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using MenusVibrant    = Menus<MenuColors::Vibrant>;
using MenuItemVibrant = Menus<MenuColors::Vibrant>::Item;

using MenusHorizontalIconOnly    = MenusSizing<MenuLayout::HorizontalIconOnly>;
using MenuItemHorizontalIconOnly = MenusSizing<MenuLayout::HorizontalIconOnly>::Item;

} // namespace ImGuiEx::M3::Spec
