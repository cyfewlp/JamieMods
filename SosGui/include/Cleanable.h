//
// Created by jamie on 2025/5/10.
//

#ifndef CLEANABLE_H
#define CLEANABLE_H

#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
class Cleanable
{
public:
    virtual ~Cleanable() = default;

    virtual void Cleanup() = 0;
};
} // namespace LIBC_NAMESPACE_DECL

#endif // CLEANABLE_H
