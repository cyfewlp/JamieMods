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
}