//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "../config.h"

#include <string_view>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{

struct Colors;

auto LoadTheme(std::string_view path, Colors &colors) -> bool;

void SetDefaultDarkTheme( Colors &colors);

}
}
