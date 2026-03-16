//
// Created by jamie on 2025/3/28.
//
#include "utils/InputFocusAnchor.h"

#include "log.h"

namespace
{
auto QueryFocusPath(const RE::GPtr<RE::GFxMovieView> &movieView, RE::GFxValue &focusPath) -> bool
{
    return movieView->Invoke("Selection.getFocus", &focusPath, nullptr, 0) && focusPath.IsString();
}

auto GetFocusMember(const RE::GPtr<RE::GFxMovieView> &movieView, const RE::GFxValue &focusPath, RE::GFxValue &focused) -> bool
{
    return movieView->GetVariable(&focused, focusPath.GetString()) && focused.IsObject();
}

void LocalToGlobalRect(const RE::GPtr<RE::GFxMovieView> &movieView, RE::GRectF &boundaries, const char *focusPath)
{
    RE::GRenderer::Matrix identity;
    RE::GPointF           screenPoint;

    const RE::GPointF leftTop{.x = boundaries.left, .y = boundaries.top};
    const RE::GPointF rightBottom{.x = boundaries.right, .y = boundaries.bottom};
    if (movieView->TranslateLocalToScreen(focusPath, leftTop, &screenPoint, &identity))
    {
        boundaries.left = screenPoint.x;
        boundaries.top  = screenPoint.y;
    }

    screenPoint = {};
    if (movieView->TranslateLocalToScreen(focusPath, rightBottom, &screenPoint, &identity))
    {
        boundaries.right  = screenPoint.x;
        boundaries.bottom = screenPoint.y;
    }
}

void GetBoundsRectFrom(const RE::GFxValue &bounds, RE::GRectF &rect)
{
    if (RE::GFxValue posX; bounds.GetMember("x", &posX) && posX.IsNumber())
    {
        rect.left = static_cast<float>(posX.GetNumber());
    }
    if (RE::GFxValue posY; bounds.GetMember("y", &posY) && posY.IsNumber())
    {
        rect.top = static_cast<float>(posY.GetNumber());
    }
    if (RE::GFxValue width; bounds.GetMember("width", &width) && width.IsNumber())
    {
        rect.right = static_cast<float>(width.GetNumber()) + rect.left;
    }
    if (RE::GFxValue height; bounds.GetMember("height", &height) && height.IsNumber())
    {
        rect.bottom = static_cast<float>(height.GetNumber() + rect.top);
    }
}

/**
 * @brief Find the first movie in the menu stack that has an active text input field
 * Search starts from the last focused menu index for efficiency, then falls back to a full search if needed.
 */
auto FindActiveInputMovie(RE::GPtr<RE::GFxMovieView> &movieView, Ime::InputFocusAnchor::size_type lastFocusedMenuIndex)
    -> Ime::InputFocusAnchor::size_type
{
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        const auto menuCount = ui->menuStack.size();
        if (lastFocusedMenuIndex < menuCount)
        {
            if (auto menu = ui->menuStack[lastFocusedMenuIndex]; menu != nullptr && menu->uiMovie != nullptr)
            {
                RE::GFxValue focusPath;
                if (QueryFocusPath(menu->uiMovie, focusPath))
                {
                    movieView = menu->uiMovie;
                    return lastFocusedMenuIndex;
                }
            }
        }

        for (auto i = menuCount - 1; i < menuCount; --i)
        {
            if (const auto menu = ui->menuStack[i]; menu != nullptr && menu->uiMovie != nullptr)
            {
                RE::GFxValue focusPath;
                if (QueryFocusPath(menu->uiMovie, focusPath))
                {
                    movieView = menu->uiMovie;
                    return i;
                }
            }
        }
        return menuCount - 1;
    }
    return Ime::InputFocusAnchor::RE_ARRAY_SIZE_MAX;
}

auto ComputeScreenMetrics(const RE::GPtr<RE::GFxMovieView> &movieView, RE::GRectF &boundariesCache) -> void
{
    if (!movieView)
    {
        return;
    }

    RE::GFxValue focusPath;
    if (!QueryFocusPath(movieView, focusPath))
    {
        return;
    }

    if (RE::GFxValue focusCharacter; GetFocusMember(movieView, focusPath, focusCharacter))
    {
        RE::GFxValue caretIndex;
        movieView->Invoke("Selection.getCaretIndex", &caretIndex, nullptr, 0);
        if (caretIndex.IsNumber())
        {
            RE::GFxValue hScroll;
            focusCharacter.GetMember("hscroll", &hScroll);
            std::array const args = {caretIndex};
            if (RE::GFxValue charBoundaries; focusCharacter.Invoke("getExactCharBoundaries", &charBoundaries, args) && charBoundaries.IsObject())
            {
                GetBoundsRectFrom(charBoundaries, boundariesCache);

                const float hScrollPos = hScroll.IsNumber() ? static_cast<float>(hScroll.GetNumber()) : 0.0F;
                boundariesCache.left -= hScrollPos;
                LocalToGlobalRect(movieView, boundariesCache, focusPath.GetString());
            }
        }
    }
}
} // namespace

void Ime::InputFocusAnchor::Reset()
{
    m_cachedBounds = {.left = 0.0F, .top = 0.0F, .right = 0.0F, .bottom = 0.0F};
}

auto Ime::InputFocusAnchor::ComputeScreenMetrics() -> void
{
    Reset();

    RE::GPtr<RE::GFxMovieView> gMovieView;
    m_lastFocusedMenuIndex = FindActiveInputMovie(gMovieView, m_lastFocusedMenuIndex);
    if (gMovieView != nullptr)
    {
        ::ComputeScreenMetrics(gMovieView, m_cachedBounds);
    }
}
