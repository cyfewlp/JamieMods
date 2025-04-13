#pragma once

#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
    class BaseGui
    {
    public:
        ~BaseGui() = default;

        virtual void Refresh() = 0;
        virtual void Close()   = 0;
    };
}