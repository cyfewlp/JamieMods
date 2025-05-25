//
// Created by jamie on 2025/3/28.
//
#include "utils/FocusGFxCharacterInfo.h"

namespace LIBC_NAMESPACE_DECL
{
using namespace RE;

auto Ime::FocusGFxCharacterInfo::Update(GFxMovieView *movieView) -> void
{
    Reset();
    m_movieView = movieView;
    if (movieView != nullptr)
    {
        GFxValue focusCharacterPath;
        movieView->Invoke("Selection.getFocus", &focusCharacterPath, nullptr, 0);
        GFxValue focusCharacter;
        if (focusCharacterPath.IsString() && movieView->GetVariable(&focusCharacter, focusCharacterPath.GetString()) &&
            focusCharacter.IsObject())
        {
            UpdateBounds(movieView, focusCharacterPath.GetString(), focusCharacter);
            UpdateTextMetrics(focusCharacter);
        }
    }
}

auto Ime::FocusGFxCharacterInfo::Update(const std::string &menuName, const bool open) -> void
{
    if (open)
    {
        m_menuStack.push_front(menuName);
        const auto movieView = UI::GetSingleton()->GetMovieView(menuName);
        Update(movieView.get());
    }
    else
    {
        std::erase(m_menuStack, menuName);
        Update(nullptr);
    }
}

auto Ime::FocusGFxCharacterInfo::UpdateByTopMenu() -> void
{
    if (m_menuStack.empty())
    {
        return;
    }
    const auto movieView = UI::GetSingleton()->GetMovieView(m_menuStack.front());
    Update(movieView.get());
}

void Ime::FocusGFxCharacterInfo::Reset()
{
    m_bound.left     = 0.0F;
    m_bound.top      = 0.0F;
    m_bound.right    = 0.0F;
    m_bound.bottom   = 0.0F;
    m_movieView      = nullptr;
    m_textWidth      = 0.0F;
    m_textHeight     = 16.0F;
    m_charBoundaries = {0.0F, 0.0F, 0.0F, 0.0F};
}

auto Ime::FocusGFxCharacterInfo::UpdateBounds(GFxMovieView *movieView, const char *path, const GFxValue &character)
    -> void
{
    GRenderer::Matrix identity;
    GPointF           screenPoint;
    if (movieView->TranslateLocalToScreen(path, {}, &screenPoint, &identity))
    {
        m_bound.left = screenPoint.x;
        m_bound.top  = screenPoint.y;
    }
    if (GFxValue _width; character.GetMember("_width", &_width) && _width.IsNumber())
    {
        m_bound.right = static_cast<float>(_width.GetNumber()) + screenPoint.x;
    }
    if (GFxValue _height; character.GetMember("_height", &_height) && _height.IsNumber())
    {
        m_bound.bottom = static_cast<float>(_height.GetNumber()) + screenPoint.y;
    }
}

auto Ime::FocusGFxCharacterInfo::UpdateTextMetrics(const GFxValue &character) -> void
{
    if (GFxValue textWidth; character.GetMember("textWidth", &textWidth) && textWidth.IsNumber())
    {
        m_textWidth = textWidth.GetNumber();
    }
    if (GFxValue textHeight; character.GetMember("textHeight", &textHeight) && textHeight.IsNumber())
    {
        m_textHeight = textHeight.GetNumber();
    }
}

auto Ime::FocusGFxCharacterInfo::UpdateCaretCharBoundaries() -> void
{
    if (!m_movieView)
    {
        UpdateByTopMenu();
    }

    if (!m_movieView)
    {
        return;
    }

    GFxValue focusCharacterPath;
    if (!m_movieView->Invoke("Selection.getFocus", &focusCharacterPath, nullptr, 0) || !focusCharacterPath.IsString())
    {
        return;
    }

    if (GFxValue focusCharacter;
        m_movieView->GetVariable(&focusCharacter, focusCharacterPath.GetString()) && focusCharacter.IsObject())
    {
        GFxValue caretIndex;
        m_movieView->Invoke("Selection.getCaretIndex", &caretIndex, nullptr, 0);
        if (caretIndex.IsNumber())
        {

            GFxValue hScroll;
            focusCharacter.GetMember("hscroll", &hScroll);
            std::array const args = {caretIndex};
            if (GFxValue charBoundaries;
                focusCharacter.Invoke("getExactCharBoundaries", &charBoundaries, args) && charBoundaries.IsObject())
            {
                const float hScrollPos = hScroll.IsNumber() ? hScroll.GetNumber() : 0.0F;
                UpdateCaretCharBoundaries(hScrollPos, charBoundaries);
            }
        }
    }
}

void Ime::FocusGFxCharacterInfo::UpdateCaretCharBoundaries(const float hScroll, const GFxValue &charBoundaries)
{
    if (GFxValue x; charBoundaries.GetMember("x", &x) && x.IsNumber())
    {
        m_charBoundaries.left = static_cast<float>(x.GetNumber()) + m_bound.left;
        m_charBoundaries.left -= hScroll;
    }
    if (GFxValue y; charBoundaries.GetMember("y", &y) && y.IsNumber())
    {
        m_charBoundaries.top = static_cast<float>(y.GetNumber()) + m_bound.top;
    }
    if (GFxValue width; charBoundaries.GetMember("width", &width) && width.IsNumber())
    {
        m_charBoundaries.right = static_cast<float>(width.GetNumber()) + m_charBoundaries.left;
    }
    if (GFxValue height; charBoundaries.GetMember("height", &height) && height.IsNumber())
    {
        m_charBoundaries.bottom = static_cast<float>(height.GetNumber()) + m_charBoundaries.top;
    }
}
}
