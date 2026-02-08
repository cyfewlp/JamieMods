//
// Created by jamie on 2026/2/8.
//

#pragma once

#include "FontManager.h"
#include "ImeWnd.hpp"
#include "common/imgui/Material3.h"

#include <vector>

namespace Ime::UI
{

auto SearchBox(ImGuiTextFilter &filter, const ImGuiEx::M3::M3Styles &m3Styles) -> bool;

/**
 * @param selectedIndex current selected @FontInof index. will be set if select a new.
 * @return is select a new row.
 */
auto FontsTable(
    FontInfo::Index &selectedIndex, const std::vector<FontInfo> &fontInfos, const ImGuiEx::M3::M3Styles &m3Styles
) -> bool;

} // namespace Ime::UI
