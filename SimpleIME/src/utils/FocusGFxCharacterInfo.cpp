//
// Created by jamie on 2025/3/28.
//
#include "utils/FocusGFxCharacterInfo.h"

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
    m_movieView      = nullptr;
    m_textWidth      = 0.0F;
    m_textHeight     = 16.0F;
    m_charBoundaries = {0.0F, 0.0F, 0.0F, 0.0F};
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
                UpdateCaretCharBoundaries(charBoundaries);
                m_charBoundaries.left -= hScrollPos;

                ConvertBoundariesToScreen(m_movieView, focusCharacterPath.GetString());
            }
        }
    }
}

void Ime::FocusGFxCharacterInfo::UpdateCaretCharBoundaries(const GFxValue &charBoundaries)
{
    if (GFxValue x; charBoundaries.GetMember("x", &x) && x.IsNumber())
    {
        m_charBoundaries.left = static_cast<float>(x.GetNumber());
    }
    if (GFxValue y; charBoundaries.GetMember("y", &y) && y.IsNumber())
    {
        m_charBoundaries.top = static_cast<float>(y.GetNumber());
    }
    if (GFxValue width; charBoundaries.GetMember("width", &width) && width.IsNumber())
    {
        m_charBoundaries.right = static_cast<float>(width.GetNumber()) + m_charBoundaries.left;
    }
    if (GFxValue height; charBoundaries.GetMember("height", &height) && height.IsNumber())
    {
        m_charBoundaries.bottom = static_cast<float>(height.GetNumber() + m_charBoundaries.top);
    }
}

void Ime::FocusGFxCharacterInfo::ConvertBoundariesToScreen(GFxMovieView *movieView, const char *path)
{
    GRenderer::Matrix identity;
    GPointF           screenPoint;

    const GPointF leftTop{m_charBoundaries.left, m_charBoundaries.top};
    const GPointF rightBottom{m_charBoundaries.right, m_charBoundaries.bottom};
    if (movieView->TranslateLocalToScreen(path, leftTop, &screenPoint, &identity))
    {
        m_charBoundaries.left = screenPoint.x;
        m_charBoundaries.top  = screenPoint.y;
    }

    screenPoint = {};
    if (movieView->TranslateLocalToScreen(path, rightBottom, &screenPoint, &identity))
    {
        m_charBoundaries.right  = screenPoint.x;
        m_charBoundaries.bottom = screenPoint.y;
    }
}
