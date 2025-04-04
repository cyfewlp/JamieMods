//
// Created by jamie on 2025/4/3.
//

#pragma once

#include "RE/A/Actor.h"
#include "SosDataType.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    class SosCallback
    {
        using UnusedTag = RE::StaticFunctionTag *;

    public:
        static auto BindPapyrusFunctions(RE::BSScript::IVirtualMachine *virtualMachine) -> bool;

        static auto GetInstance() -> SosCallback &
        {
            static SosCallback instance;
            return instance;
        }

    private:
        static auto SetListActors(UnusedTag, std::vector<RE::Actor *> actors) -> void;
        static auto SetActorNearPC(UnusedTag, std::vector<RE::Actor *> actors) -> void;
        static auto SetIsEnabled(UnusedTag, bool isEnabled) -> void;
        static auto SetAutoSwitchEnabled(UnusedTag, RE::Actor *actor, bool isEnabled) -> void;
        static auto SetActorOutfitState(UnusedTag, RE::Actor *actor, StateType stateType, std::string_view state)
            -> void;
        static auto SetOutfitList(UnusedTag, std::vector<std::string_view> outfitList) -> void;
        static auto SetArmorCandidates(UnusedTag, std::vector<RE::TESObjectARMO *> armorCandidates) -> void;
        static void SetOutfitArmors(UnusedTag, std::string_view outfitName, std::vector<int32_t> slots,
                                    std::vector<RE::TESObjectARMO *> armors);
    };
}