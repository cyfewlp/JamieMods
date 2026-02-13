//
// Created by jamie on 2026/2/2.
//
#include "M3ThemeBuilder.h"
#include "Material3.h"

#include <cmath>

namespace ImGuiEx::M3
{
void M3Styles::UpdateTypeScaleScaling(const float newScale) const
{
    m_cachedTypeScale.currText.textSize   = m_cachedTypeScale.unScaledText.textSize * newScale;
    m_cachedTypeScale.currText.lineHeight = m_cachedTypeScale.unScaledText.lineHeight * newScale;
    m_cachedTypeScale.currHalfLineGap     = HalfLineGap(m_cachedTypeScale.currText);
}

void M3Styles::UpdateScaling(const float newScale)
{
    if (m_currentScale == newScale)
    {
        return;
    }
    m_currentScale = newScale;
    for (size_t i = 0U; i < precomputedPx.size(); ++i)
    {
        precomputedPx[i] = std::floor(static_cast<float>(i * Spec::BASE_UNIT) * newScale);
    }
    UpdateTypeScaleScaling(m_currentScale);
    labelText.textSize   = TEXT_LABEL_LARGE.textSize * newScale;
    labelText.lineHeight = TEXT_LABEL_LARGE.lineHeight * newScale;

    iconSize = ICON_SIZE * newScale;
}

void M3Styles::RebuildColors(const Colors::SchemeConfig &schemeConfig)
{
    colors = ThemeBuilder::Build(schemeConfig);
}
} // namespace ImGuiEx::M3
