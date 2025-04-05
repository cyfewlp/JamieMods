//
// Created by jamie on 2025/4/3.
//
#include "SosCallback.h"
#include "PapyrusEvent.h"
#include "SosUiData.h"

#include "common/log.h"

namespace LIBC_NAMESPACE_DECL
{
    auto SosCallback::SetListActors(UnusedTag, std::vector<RE::Actor *> actors) -> void
    {
        log_debug("SosCallback::{}", __func__);
        SosUiData::GetInstance().SetActors(actors);
    }

    auto SosCallback::SetActorNearPC(UnusedTag, std::vector<RE::Actor *> actors) -> void
    {
        log_debug("SosCallback::{}", __func__);
        SosUiData::GetInstance().SetNearActors(actors);
    }

    auto SosCallback::SetIsEnabled(UnusedTag, bool isEnabled) -> void
    {
        SosUiData::GetInstance().SetEnabled(isEnabled);
    }

    auto SosCallback::SetAutoSwitchEnabled(UnusedTag, RE::Actor *actor, bool isEnabled) -> void
    {
        SosUiData::GetInstance().SetAutoSwitchEnabled(actor, isEnabled);
        if (isEnabled)
        {
            PapyrusEvent::GetInstance().CallGetOutfitState(actor, StateType_All);
        }
    }

    auto SosCallback::SetActorOutfitState(UnusedTag, RE::Actor *actor, StateType stateType, std::string state) -> void
    {
        SosUiData::GetInstance().PutActorOutfitState(actor, {stateType, state});
    }

    auto SosCallback::SetOutfitList(UnusedTag, std::vector<std::string> outfitList) -> void
    {
        SosUiData::GetInstance().SetOutfitList(outfitList);
    }

    auto SosCallback::SetArmorCandidates(UnusedTag, std::vector<RE::TESObjectARMO *> armorCandidates) -> void
    {
        SosUiData::GetInstance().SetArmorCandidates(armorCandidates);
    }

    void SosCallback::SetOutfitArmors(UnusedTag, std::string outfitName, std::vector<int32_t> slots,
                                      std::vector<RE::TESObjectARMO *> armors)
    {
        SosUiData::GetInstance().SetOutfitBodySlotArmors(outfitName, slots, armors);
    }

    auto SosCallback::BindPapyrusFunctions(RE::BSScript::IVirtualMachine *vm) -> bool
    {
        REGISTER_FUNCTION(SetListActors);
        REGISTER_FUNCTION(SetActorNearPC);
        REGISTER_FUNCTION(SetIsEnabled);
        REGISTER_FUNCTION(SetAutoSwitchEnabled);
        REGISTER_FUNCTION(SetActorOutfitState);
        REGISTER_FUNCTION(SetOutfitList);
        REGISTER_FUNCTION(SetArmorCandidates);
        REGISTER_FUNCTION(SetOutfitArmors);
        return true;
    }
}