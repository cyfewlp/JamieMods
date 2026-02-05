//
// Created by jamie on 2025/3/27.
//

#pragma once
#include "RE/G/GRect.h"

namespace Ime
{
class FocusGFxCharacterInfo
{
    RE::GRectF                 m_charBoundaries{};
    RE::GPtr<RE::GFxMovieView> m_currMovieView{nullptr};

public:
    [[nodiscard]] auto CharBoundaries() const -> const RE::GRectF &
    {
        return m_charBoundaries;
    }

    static auto GetInstance() -> FocusGFxCharacterInfo &
    {
        static FocusGFxCharacterInfo instance;
        return instance;
    }

    /// Call when
    /// 1. Exists text entry
    /// 2. SKSE_AllowTextInput/Scaleform_AllowTextInput called
    auto Update(RE::GFxMovieView *movieView) -> void;

    auto UpdateByTopMenu() -> void;
    auto UpdateCaretCharBoundaries() -> void;

private:
    void Reset();
};
} // namespace Ime
