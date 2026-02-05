//
// Created by jamie on 2025/3/28.
//
#include "utils/FocusGFxCharacterInfo.h"

namespace
{
/// Console menu depth: CursorMenu, ImeMenu depth is 13.
constexpr uint32_t MENU_MAX_EFFECTIVE_DEPTH = 12;

auto GetFocusPath(const RE::GPtr<RE::GFxMovieView> &movieView, RE::GFxValue &focusPath) -> bool
{
    return movieView->Invoke("Selection.getFocus", &focusPath, nullptr, 0) && focusPath.IsString();
}

auto GetFocusCharacter(
    const RE::GPtr<RE::GFxMovieView> &movieView, const RE::GFxValue &focusPath, RE::GFxValue &focused
) -> bool
{
    return movieView->GetVariable(&focused, focusPath.GetString()) && focused.IsObject();
}

void ConvertBoundariesToScreen(
    const RE::GPtr<RE::GFxMovieView> &movieView, RE::GRectF boundaries, const char *focusPath
)
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

void GetBoundariesFrom(const RE::GFxValue &charBoundaries, RE::GRectF &gBoundaries)
{
    if (RE::GFxValue posX; charBoundaries.GetMember("x", &posX) && posX.IsNumber())
    {
        gBoundaries.left = static_cast<float>(posX.GetNumber());
    }
    if (RE::GFxValue posY; charBoundaries.GetMember("y", &posY) && posY.IsNumber())
    {
        gBoundaries.top = static_cast<float>(posY.GetNumber());
    }
    if (RE::GFxValue width; charBoundaries.GetMember("width", &width) && width.IsNumber())
    {
        gBoundaries.right = static_cast<float>(width.GetNumber()) + gBoundaries.left;
    }
    if (RE::GFxValue height; charBoundaries.GetMember("height", &height) && height.IsNumber())
    {
        gBoundaries.bottom = static_cast<float>(height.GetNumber() + gBoundaries.top);
    }
}
} // namespace

auto Ime::FocusGFxCharacterInfo::Update(RE::GFxMovieView *movieView) -> void
{
    Reset();
    m_currMovieView = RE::GPtr(movieView);
}

auto Ime::FocusGFxCharacterInfo::UpdateByTopMenu() -> void
{
    RE::IMenu *pMenu = nullptr; // NOLINT(*-const-correctness)
    RE::UI::GetSingleton()->GetTopMostMenu(&pMenu, MENU_MAX_EFFECTIVE_DEPTH);
    if (pMenu != nullptr)
    {
        Update(pMenu->uiMovie.get());
    }
}

void Ime::FocusGFxCharacterInfo::Reset()
{
    m_currMovieView  = nullptr;
    m_charBoundaries = {.left = 0.0F, .top = 0.0F, .right = 0.0F, .bottom = 0.0F};
}

auto Ime::FocusGFxCharacterInfo::UpdateCaretCharBoundaries() -> void
{
    if (!m_currMovieView)
    {
        return;
    }

    RE::GFxValue focusPath;
    if (!GetFocusPath(m_currMovieView, focusPath))
    {
        return;
    }

    if (RE::GFxValue focusCharacter; GetFocusCharacter(m_currMovieView, focusPath, focusCharacter))
    {
        RE::GFxValue caretIndex;
        m_currMovieView->Invoke("Selection.getCaretIndex", &caretIndex, nullptr, 0);
        if (caretIndex.IsNumber())
        {
            RE::GFxValue hScroll;
            focusCharacter.GetMember("hscroll", &hScroll);
            std::array const args = {caretIndex};
            if (RE::GFxValue charBoundaries;
                focusCharacter.Invoke("getExactCharBoundaries", &charBoundaries, args) && charBoundaries.IsObject())
            {
                GetBoundariesFrom(charBoundaries, m_charBoundaries);

                const float hScrollPos = hScroll.IsNumber() ? static_cast<float>(hScroll.GetNumber()) : 0.0F;
                m_charBoundaries.left -= hScrollPos;
                ConvertBoundariesToScreen(m_currMovieView, m_charBoundaries, focusPath.GetString());
            }
        }
    }
}
