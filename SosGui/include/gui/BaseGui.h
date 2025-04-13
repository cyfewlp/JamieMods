#pragma once

#include "SosDataType.h"
#include "common/config.h"
#include <RE/A/Actor.h>

namespace LIBC_NAMESPACE_DECL
{
    struct GuiContext
    {
        RE::Actor *editingActor = nullptr;
        StateType  editingState = StateType::None;
    };

    class BaseGui
    {
    public:
        ~BaseGui() = default;

        virtual void Refresh() = 0;
        virtual void Close()   = 0;
    };
}