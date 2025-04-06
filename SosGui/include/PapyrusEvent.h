#pragma once
#include "SosDataType.h"

namespace LIBC_NAMESPACE_DECL
{
#define REGISTER_FUNCTION(name) vm->RegisterFunction(#name, "SosGuiNative", name)

#define ADD_NEW_EVENT(funcName, ...)                                                                                   \
    SKSE::RegistrationSet<__VA_ARGS__> require##funcName{"OnRequire" #funcName};                                       \
    static void                        RegisterForRequire##funcName(RE::StaticFunctionTag *, RE::TESForm *thisForm)    \
    {                                                                                                                  \
        GetInstance().require##funcName.Register(thisForm);                                                            \
    }

    class PapyrusEvent
    {
        using Armor = RE::TESObjectARMO;

        SKSE::RegistrationSet<std::string>       requireUiDataEvent{"OnRequireUiData"};
        SKSE::RegistrationSet<const RE::Actor *> requireAddActor{"OnRequireAddActor"};
        SKSE::RegistrationSet<bool>              requireSetEnabled{"OnRequireSetEnabled"};
        ADD_NEW_EVENT(RemoveActor, const RE::Actor *)
        ADD_NEW_EVENT(SetAutoSwitchEnabled, const RE::Actor *, bool)
        ADD_NEW_EVENT(GetAutoSwitchEnabled, const RE::Actor *)
        ADD_NEW_EVENT(GetOutfitState, const RE::Actor *, StateType)
        ADD_NEW_EVENT(GetOutfitList)
        ADD_NEW_EVENT(CreateOutfit, std::string, bool)
        ADD_NEW_EVENT(RenameOutfit, std::string, std::string)
        ADD_NEW_EVENT(GetOutfitArmors, std::string)
        ADD_NEW_EVENT(SetQuickslot, bool) // add spell

        ADD_NEW_EVENT(GetActorArmors, const RE::Actor *, OutfitAddPolicy)
        ADD_NEW_EVENT(AddToOutfit, std::string, const Armor *)
        ADD_NEW_EVENT(SwapArmor, std::string, const Armor *)

    public:
        static auto Bind(RE::BSScript::IVirtualMachine *vm) -> bool;

        static auto GetInstance() -> PapyrusEvent &
        {
            static PapyrusEvent g_instance;
            return g_instance;
        }

        constexpr auto CallNoArgs(const std::string &methodName)
        {
            requireUiDataEvent.QueueEvent(methodName);
        }

        constexpr auto CallAddActor(const RE::Actor *actor)
        {
            requireAddActor.QueueEvent(actor);
        }

        constexpr auto CallSetEnabled(bool enabled)
        {
            requireSetEnabled.QueueEvent(enabled);
        }

        constexpr auto CallRemoveActor(const RE::Actor *actor)
        {
            requireRemoveActor.QueueEvent(actor);
        }

        constexpr auto CallSetAutoSwitchEnabled(const RE::Actor *actor, bool enable)
        {
            requireSetAutoSwitchEnabled.QueueEvent(actor, enable);
        }

        constexpr auto CallGetAutoSwitchEnabled(const RE::Actor *actor)
        {
            requireGetAutoSwitchEnabled.QueueEvent(actor);
        }

        constexpr auto CallGetOutfitState(const RE::Actor *actor, StateType state)
        {
            requireGetOutfitState.QueueEvent(actor, state);
        }

        constexpr auto CallGetOutfitList()
        {
            requireGetOutfitList.QueueEvent();
        }

        auto CallCreateOutfit(const std::string &outfitName, bool fromWorn)
        {
            requireCreateOutfit.QueueEvent(outfitName, fromWorn);
        }

        auto CallRenameOutfit(const std::string &outfitName, const std::string &newName)
        {
            requireRenameOutfit.QueueEvent(outfitName, newName);
        }

        constexpr auto CallGetActorArmors(const RE::Actor *actor, OutfitAddPolicy policy)
        {
            requireGetActorArmors.QueueEvent(actor, policy);
        }

        auto CallAddToOutfit(const std::string &outfitName, const Armor *armor)
        {
            requireAddToOutfit.QueueEvent(outfitName, armor);
        }

        auto CallGetOutfitArmors(const std::string &outfitName)
        {
            requireGetOutfitArmors.SendEvent(outfitName);
        }

        constexpr auto CallSetQuickslot(bool enable)
        {
            requireSetQuickslot.QueueEvent(enable);
        }

        constexpr auto CallSwapArmor(const std::string &outfitName, const Armor *armor)
        {
            requireSwapArmor.QueueEvent(outfitName, armor);
        }

    private:
        static void RegisterForRequireUiData(RE::StaticFunctionTag *, const RE::TESForm *thisForm)
        {
            GetInstance().requireUiDataEvent.Register(thisForm);
        }

        static void RegisterForRequireAddActor(RE::StaticFunctionTag *, const RE::TESForm *thisForm)
        {
            GetInstance().requireAddActor.Register(thisForm);
        }

        static void RegisterForRequireSetEnabled(RE::StaticFunctionTag *, const RE::TESForm *thisForm)
        {
            GetInstance().requireSetEnabled.Register(thisForm);
        }
    };
}