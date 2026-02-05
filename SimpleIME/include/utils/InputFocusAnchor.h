//
// Created by jamie on 2025/3/27.
//

#pragma once
#include "RE/G/GRect.h"

namespace Ime
{
class InputFocusAnchor
{
    RE::GRectF m_cachedBounds{};

public:
    [[nodiscard]] auto GetLastBounds() const -> const RE::GRectF &
    {
        return m_cachedBounds;
    }

    /**
     * Find active input editor and compute text metrics.
     */
    auto ComputeScreenMetrics() -> void;

    static auto GetInstance() -> InputFocusAnchor &
    {
        static InputFocusAnchor instance;
        return instance;
    }

private:
    void Reset();
};
} // namespace Ime
