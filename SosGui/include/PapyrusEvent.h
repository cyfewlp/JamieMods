#pragma once
#include "SosDataType.h"

namespace LIBC_NAMESPACE_DECL
{
#define REGISTER_FUNCTION(name) vm->RegisterFunction(#name, "SosGuiNative", name)

#define ADD_NEW_EVENT(funcName, type...)                                                                               \
    SKSE::RegistrationSet<type> require##funcName{"OnRequire" #funcName};                                              \
    static void                 RegisterForRequire##funcName(RE::StaticFunctionTag *, RE::TESForm *thisForm)           \
    {                                                                                                                  \
        GetInstance().require##funcName.Register(thisForm);                                                            \
    }

    class PapyrusEvent
    {
        using String = const char *;
        SKSE::RegistrationSet<String>            requireUiDataEvent{"OnRequireUiData"};
        SKSE::RegistrationSet<const RE::Actor *> requireAddActor{"OnRequireAddActor"};
        SKSE::RegistrationSet<bool>              requireSetEnabled{"OnRequireSetEnabled"};
        ADD_NEW_EVENT(RemoveActor, const RE::Actor *)
        ADD_NEW_EVENT(SetAutoSwitchEnabled, const RE::Actor *, bool)
        ADD_NEW_EVENT(GetAutoSwitchEnabled, const RE::Actor *)
        ADD_NEW_EVENT(GetOutfitState, const RE::Actor *, StateType)
        ADD_NEW_EVENT(GetOutfitList)
        ADD_NEW_EVENT(CreateOutfit, String, bool)
        ADD_NEW_EVENT(RenameOutfit, String, String)
        ADD_NEW_EVENT(GetOutfitArmors, String)
        ADD_NEW_EVENT(SetQuickslot, bool) // add spelll

        ADD_NEW_EVENT(GetActorArmors, const RE::Actor *, OutfitAddPolicy)
        ADD_NEW_EVENT(AddToOutfit, String, const RE::TESObjectARMO *)

    public:
        static auto Bind(RE::BSScript::IVirtualMachine *vm) -> bool;

        static auto GetInstance() -> PapyrusEvent &
        {
            static PapyrusEvent g_instance;
            return g_instance;
        }

        constexpr auto CallNoArgs(String methodName)
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

        constexpr auto CallCreateOutfit(String outfitName, bool fromWorn)
        {
            requireCreateOutfit.QueueEvent(outfitName, fromWorn);
        }

        constexpr auto CallRenameOutfit(String outfitName, String newName)
        {
            requireRenameOutfit.QueueEvent(outfitName, newName);
        }

        constexpr auto CallGetActorArmors(const RE::Actor *actor, OutfitAddPolicy policy)
        {
            requireGetActorArmors.QueueEvent(actor, policy);
        }

        constexpr auto CallAddToOutfit(String outfitName, const RE::TESObjectARMO *armor)
        {
            requireAddToOutfit.QueueEvent(outfitName, armor);
        }

        constexpr auto CallGetOutfitArmors(String outfitName)
        {
            requireGetOutfitArmors.QueueEvent(outfitName);
        }

        constexpr auto CallSetQuickslot(bool enable)
        {
            requireSetQuickslot.QueueEvent(enable);
        }

    private:
        static void RegisterForRequireUiData(RE::StaticFunctionTag *, RE::TESForm *thisForm)
        {
            GetInstance().requireUiDataEvent.Register(thisForm);
        }

        static void RegisterForRequireAddActor(RE::StaticFunctionTag *, RE::TESForm *thisForm)
        {
            GetInstance().requireAddActor.Register(thisForm);
        }

        static void RegisterForRequireSetEnabled(RE::StaticFunctionTag *, RE::TESForm *thisForm)
        {
            GetInstance().requireSetEnabled.Register(thisForm);
        }
    };
}