#pragma once

#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
    class BaseGui
    {
    public:
        virtual ~BaseGui() = default;

        virtual void Refresh() = 0;
        virtual void Close()   = 0;
    };
}