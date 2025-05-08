#pragma once

#include "common/config.h"

namespace
LIBC_NAMESPACE_DECL
{
class BaseGui
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

    void Show()
    {
        m_show = true;
    }

    void Hide()
    {
        m_show = false;
    }

    bool IsShowing() const
    {
        return m_show;
    }

    virtual void Refresh() = 0;
    virtual void Close() = 0;

protected:
    bool m_show = false;
};
}