//
// Created by jamie on 2026/2/2.
//
#include "M3ThemeBuilder.h"
#include "Material3.h"
#include "log.h"

#include <cmath>

namespace ImGuiEx::M3
{
void M3Styles::UpdateTypeScaleScaling(detail::CachedTypeScale &cachedTypeScale, const float newScale)
{
    cachedTypeScale.currText.textSize   = cachedTypeScale.unScaledText.textSize * newScale;
    cachedTypeScale.currText.lineHeight = cachedTypeScale.unScaledText.lineHeight * newScale;
    cachedTypeScale.currHalfLineGap     = HalfLineGap(cachedTypeScale.currText);
}

void M3Styles::UpdateScaling(const float newScale)
{
    if (m_currentScale == newScale)
    {
        return;
    }
    m_currentScale = newScale;
    for (size_t i = 0U; i < m_precomputedPx.size(); ++i)
    {
        m_precomputedPx[i] = std::floor(static_cast<float>(i * Spec::BASE_UNIT) * newScale);
    }
    UpdateTypeScaleScaling(m_cachedTypeScale, m_currentScale);
}

void M3Styles::RebuildColors(const ColorScheme::SchemeConfig &schemeConfig)
{
    m_scheme = ThemeBuilder::Build(schemeConfig);
}

namespace
{
std::unique_ptr<M3Styles> g_currentStyles = nullptr;
}

auto Context::CreateM3Styles(ImFont *iconFont, const ColorScheme::SchemeConfig &schemeConfig) -> M3Styles &
{
    auto colors     = ImGuiEx::M3::ThemeBuilder::Build(schemeConfig);
    g_currentStyles = std::make_unique<ImGuiEx::M3::M3Styles>(colors, iconFont);

    return *g_currentStyles;
}

auto Context::DestroyM3Styles() -> void
{
    g_currentStyles.reset();
}

auto Context::GetM3Styles() -> M3Styles &
{
    assert(g_currentStyles.get() != nullptr && "Did you forget to call Context::CreateM3Styles() before using M3 styles?");
    return *g_currentStyles;
}
} // namespace ImGuiEx::M3
