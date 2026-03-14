//
// Created by jamie on 2025/3/27.
//

#pragma once
#include "RE/G/GRect.h"

namespace Ime
{
class InputFocusAnchor
{
public:
    using size_type                              = RE::BSTArray<RE::IMenu>::size_type;
    static constexpr size_type RE_ARRAY_SIZE_MAX = std::numeric_limits<size_type>::max();

    [[nodiscard]] auto GetLastBounds() const -> const RE::GRectF & { return m_cachedBounds; }

    /**
     * @brief Computes the screen coordinates of the active text input field.
     * Compute from the provided @p movieView or by searching through the menu stack if @p movieView is null.
     */
    auto ComputeScreenMetrics() -> void;

    static auto GetInstance() -> InputFocusAnchor &
    {
        static InputFocusAnchor instance;
        return instance;
    }

private:
    void Reset();

    RE::GRectF m_cachedBounds{};
    size_type  m_lastFocusedMenuIndex{RE_ARRAY_SIZE_MAX};
};
} // namespace Ime
