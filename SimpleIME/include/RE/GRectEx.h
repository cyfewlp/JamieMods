//
// Created by jamie on 2026/2/6.
//
#pragma once

#include "RE/G/GRect.h"

namespace RE
{

template <typename T>
struct GSize
{
    T width;
    T height;
};

using GSizeF = GSize<float>;

template <class T>
auto Intersects(GRect<T> left, const GRect<T> &r) -> bool
{
    if (left.bottom >= r.top && r.bottom >= left.top)
    {
        return left.right >= r.left && r.right >= left.left;
    }
    return false;
}

} // namespace RE
