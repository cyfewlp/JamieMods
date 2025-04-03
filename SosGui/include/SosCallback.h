//
// Created by jamie on 2025/4/3.
//

#pragma once

#include "PapyrusEvent.h"
#include "RE/A/Actor.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    class SosCallback
    {
    public:
        static auto BindPapyrusFunctions(RE::BSScript::IVirtualMachine *virtualMachine) -> bool;

        static auto GetInstance() -> SosCallback &
        {
            static SosCallback instance;
            return instance;
        }

    private:
        static auto SetListActors(RE::StaticFunctionTag *, std::vector<RE::Actor *> actors) -> void;
        static auto SetActorNearPC(RE::StaticFunctionTag *, std::vector<RE::Actor *> actors) -> void;
        static auto SetIsEnabled(RE::StaticFunctionTag *, bool isEnabled) -> void;
    };
}