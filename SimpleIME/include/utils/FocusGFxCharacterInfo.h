//
// Created by jamie on 2025/3/27.
//

#pragma once
#include "RE/G/GRect.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class FocusGFxCharacterInfo
        {
            RE::GRectF        m_bound     = {};
            RE::GFxMovieView *m_movieView = nullptr;
            float             m_textHeight;
            float             m_textWidth;
            RE::GRectF        m_charBoundaries;

            FocusGFxCharacterInfo()  = default;
            ~FocusGFxCharacterInfo() = default;

        public:
            auto GetBound() const -> const RE::GRectF &
            {
                return m_bound;
            }

            [[nodiscard]] float TextHeight() const
            {
                return m_textHeight;
            }

            [[nodiscard]] float TextWidth() const
            {
                return m_textWidth;
            }

            [[nodiscard]] auto Bound() const -> const RE::GRectF &
            {
                return m_bound;
            }

            [[nodiscard]] auto MovieView() const -> RE::GFxMovieView *
            {
                return m_movieView;
            }

            [[nodiscard]] auto CharBoundaries() const -> const RE::GRectF &
            {
                return m_charBoundaries;
            }

            constexpr auto IsValid() const -> bool
            {
                return m_movieView != nullptr;
            }

            static auto GetInstance() -> FocusGFxCharacterInfo &
            {
                static FocusGFxCharacterInfo instance;
                return instance;
            }

            // Call when
            // 1. Exists text entry
            // 2. Any menu open/close (expect Cursor Menu * HUD Menu)
            // 3. SKSE_AllowTextInput/Scaleform_AllowTextInput called
            auto Update(RE::GFxMovieView *movieView) -> void;
            auto UpdateCaretCharBoundaries() -> void;

        private:
            void Reset();
            auto UpdateBounds(RE::GFxMovieView *movieView, const char *path, const RE::GFxValue &character) -> void;
            auto UpdateTextMetrics(const RE::GFxValue &character) -> void;
        };
    }
}