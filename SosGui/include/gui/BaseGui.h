#pragma once

#include "Cleanable.h"
#include "common/config.h"

namespace
LIBC_NAMESPACE_DECL
{
class BaseGui : public Cleanable
{
public:
    static constexpr float DEFAULT_WINDOW_POS_X = 100;
    static constexpr float DEFAULT_WINDOW_POS_Y = 100;

    static constexpr float DEFAULT_WINDOW_HEIGHT      = 680;
    static constexpr float DEFAULT_WINDOW_WIDTH       = 500;
    static constexpr float DEFAULT_WINDOW_WIDTH_SMALL = 200;

    static constexpr float DEFAULT_OUTFIT_LIST_WINDOW_POS_X = DEFAULT_WINDOW_POS_X;
    static constexpr float DEFAULT_MAIN_WINDOW_POS_X        =
        DEFAULT_OUTFIT_LIST_WINDOW_POS_X + DEFAULT_WINDOW_WIDTH_SMALL + 10;
    static constexpr float DEFAULT_OUTFIT_EDIT_WINDOW_POS_X = DEFAULT_MAIN_WINDOW_POS_X + DEFAULT_WINDOW_WIDTH + 10;

    virtual ~BaseGui() = default;

    virtual void Show()
    {
        m_show = true;
    }

    virtual void OnRefresh()
    {

    }

    void Hide()
    {
        m_show = false;
    }

    void ToggleShow()
    {
        m_show = !m_show;
    }

    bool IsShowing() const
    {
        return m_show;
    }

protected:
    bool m_show = false;
};
}