//
// Created by jamie on 2025/3/28.
//
#include "utils/FocusGFxCharacterInfo.h"

#include <common/log.h>

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
            if (focusCharacterPath.IsString() &&
                movieView->GetVariable(&focusCharacter, focusCharacterPath.GetString()) && focusCharacter.IsObject())
            {
                UpdateBounds(movieView, focusCharacterPath.GetString(), focusCharacter);
                UpdateTextMetrics(focusCharacter);
            }
        }
    }

    auto Ime::FocusGFxCharacterInfo::Update(const std::string &menuName, bool open) -> void
    {
        if (open)
        {
            m_menuStack.push_front(menuName);
            auto movieView = UI::GetSingleton()->GetMovieView(menuName);
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
        if (movieView == nullptr)
        {
            return;
        }
        GRenderer::Matrix identity;
        GPointF           screenPoint;
        if (movieView->TranslateLocalToScreen(path, {}, &screenPoint, &identity))
        {
            m_bound.left = screenPoint.x;
            m_bound.top  = screenPoint.y;
        }
        GFxValue _width, _height;
        if (character.GetMember("_width", &_width) && _width.IsNumber())
        {
            m_bound.right = static_cast<float>(_width.GetNumber()) + screenPoint.x;
        }
        if (character.GetMember("_height", &_height) && _height.IsNumber())
        {
            m_bound.bottom = static_cast<float>(_height.GetNumber()) + screenPoint.y;
        }
    }

    auto Ime::FocusGFxCharacterInfo::UpdateTextMetrics(const GFxValue &character) -> void
    {
        GFxValue textWidth, textHeight;
        if (character.GetMember("textWidth", &textWidth) && textWidth.IsNumber())
        {
            m_textWidth = textWidth.GetNumber();
        }
        if (character.GetMember("textHeight", &textHeight) && textHeight.IsNumber())
        {
            m_textHeight = textHeight.GetNumber();
        }
    }

    auto Ime::FocusGFxCharacterInfo::UpdateCaretCharBoundaries() -> void
    {
        if (m_movieView == nullptr)
        {
            UpdateByTopMenu();
        }

        if (m_movieView != nullptr)
        {
            GFxValue focusCharacterPath;
            m_movieView->Invoke("Selection.getFocus", &focusCharacterPath, nullptr, 0);
            GFxValue focusCharacter;
            if (focusCharacterPath.IsString() &&
                m_movieView->GetVariable(&focusCharacter, focusCharacterPath.GetString()) && focusCharacter.IsObject())
            {
                GFxValue caretIndex;
                m_movieView->Invoke("Selection.getCaretIndex", &caretIndex, nullptr, 0);
                if (caretIndex.IsNumber())
                {
                    GFxValue charBoundaries, hscroll;
                    focusCharacter.GetMember("hscroll", &hscroll);
                    std::array args = {caretIndex};
                    focusCharacter.Invoke("getExactCharBoundaries", &charBoundaries, args);
                    if (charBoundaries.IsObject())
                    {
                        GFxValue x, y, width, height;
                        charBoundaries.GetMember("x", &x);
                        charBoundaries.GetMember("y", &y);
                        charBoundaries.GetMember("width", &width);
                        charBoundaries.GetMember("height", &height);
                        if (x.IsNumber())
                        {
                            m_charBoundaries.left = static_cast<float>(x.GetNumber()) + m_bound.left;
                            if (hscroll.IsNumber())
                            {
                                m_charBoundaries.left -= static_cast<float>(hscroll.GetNumber());
                            }
                        }
                        if (y.IsNumber())
                        {
                            m_charBoundaries.top = static_cast<float>(y.GetNumber()) + m_bound.top;
                        }
                        if (width.IsNumber())
                        {
                            m_charBoundaries.right = static_cast<float>(width.GetNumber()) + m_charBoundaries.left;
                        }
                        if (height.IsNumber())
                        {
                            m_charBoundaries.bottom = static_cast<float>(height.GetNumber()) + m_charBoundaries.top;
                        }
                    }
                }
            }
        }
    }
}
