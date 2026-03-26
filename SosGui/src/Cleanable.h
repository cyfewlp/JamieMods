//
// Created by jamie on 2025/5/10.
//
#pragma once

namespace SosGui
{
class Cleanable
{
public:
    virtual ~Cleanable() = default;

    virtual void Cleanup() = 0;
};
} // namespace SosGui
