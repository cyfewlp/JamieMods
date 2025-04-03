//
// Created by jamie on 2025/4/3.
//
#include "SosCallback.h"
#include "SosUiData.h"

#include "common/log.h"

namespace LIBC_NAMESPACE_DECL
{
    constexpr auto CLASS_NAME = "SosGuiNative"sv;

    auto SosCallback::SetListActors(RE::StaticFunctionTag *, std::vector<RE::Actor *> actors) -> void
    {
        log_debug("SosCallback::{}", __func__);
        SosUiData::GetInstance().SetActors(actors);
    }

    auto SosCallback::SetActorNearPC(RE::StaticFunctionTag *, std::vector<RE::Actor *> actors) -> void
    {
        log_debug("SosCallback::{}", __func__);
        SosUiData::GetInstance().SetNearActors(actors);
    }

    auto SosCallback::SetIsEnabled(RE::StaticFunctionTag *, bool isEnabled) -> void
    {
        SosUiData::GetInstance().SetEnabled(isEnabled);
    }

    auto SosCallback::BindPapyrusFunctions(RE::BSScript::IVirtualMachine *virtualMachine) -> bool
    {
        virtualMachine->RegisterFunction("SetListActors", CLASS_NAME, SetListActors);
        virtualMachine->RegisterFunction("SetActorNearPC", CLASS_NAME, SetActorNearPC);
        virtualMachine->RegisterFunction("SetIsEnabled", CLASS_NAME, SetIsEnabled);
        return true;
    }
}