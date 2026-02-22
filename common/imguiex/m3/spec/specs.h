//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "button.h"
#include "menu.h"
#include "nav_rail.h"
#include "others.h"
#include "slider.h"
#include "text_field.h"

/**
 * This namespace defines size specifications for various Material Design 3 components.
 * All sizes value directly reference from Material3 design guidelines, represented in grid units (**4dp** each).
 * But, some size already optimized to better fit ImGui usage.
 * Such as, the FontSize will not contain the line height part. There will combine to padding or gap when needed.
 *
 * ## Usages
 * For follow the guidelines, you **SHOULD NOT** specify the size in pixels directly.
 *
 * If you want to scale components, it is recommended to call the ImGuiEx::M3::M3Styles::UpdateScaling method to update
 * the scale factor, which will automatically adjust all sizes accordingly.
 *
 * ## Layout Logic for Text and Components
 * @details
 * - **Pixel-Based Sizing:** Font sizes use Pixels because they often don't align with grid units.
 * - **Grid Alignment:** We ensure the final component height (not the font) is a multiple of the unit.
 * - **Runtime Padding:** To maintain alignment without presets, padding is computed dynamically:
 * @code paddingY = (buttonHeight - fontSize) / 2 @endcode
 * @note ImGui lacks native line-height support; therefore, line-height reference values
 * are generally integrated into the component's padding.
 */
namespace ImGuiEx::M3::Spec
{

}
