//
// Created by jamie on 2025/3/28.
//
#include "utils/InputFocusAnchor.h"

#include "common/log.h"

namespace
{
/// Console menu depth: CursorMenu, ImeMenu depth is 13.
constexpr int8_t MENU_MAX_EFFECTIVE_DEPTH = 12;

auto QueryFocusPath(const RE::GPtr<RE::GFxMovieView> &movieView, RE::GFxValue &focusPath) -> bool
{
    return movieView->Invoke("Selection.getFocus", &focusPath, nullptr, 0) && focusPath.IsString();
}

auto GetFocusMember(const RE::GPtr<RE::GFxMovieView> &movieView, const RE::GFxValue &focusPath, RE::GFxValue &focused)
    -> bool
{
    return movieView->GetVariable(&focused, focusPath.GetString()) && focused.IsObject();
}

void LocalToGlobalRect(const RE::GPtr<RE::GFxMovieView> &movieView, RE::GRectF boundaries, const char *focusPath)
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

/// Get the first menu that has active input editor.
/// The menu that depth priority great 12 (Console) will be ignored.
/// @param movieView the result
auto FindActiveInputMovie(RE::GPtr<RE::GFxMovieView> &movieView) -> void
{
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        for (size_t i = ui->menuStack.size() - 1; i > 0; --i)
        {
            auto &menu = ui->menuStack[i];
            if (menu->depthPriority <= MENU_MAX_EFFECTIVE_DEPTH && menu->uiMovie != nullptr)
            {
                movieView = menu->uiMovie;
                break;
            }
        }
    }
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
            if (RE::GFxValue charBoundaries;
                focusCharacter.Invoke("getExactCharBoundaries", &charBoundaries, args) && charBoundaries.IsObject())
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

    RE::GPtr<RE::GFxMovieView> movieView;
    FindActiveInputMovie(movieView);
    if (movieView)
    {
        ::ComputeScreenMetrics(movieView, m_cachedBounds);
    }
}
