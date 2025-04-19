#pragma once

#include "SosDataType.h"
#include "common/config.h"

#include <RE/A/Actor.h>
#include <coroutine>
#include <string>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    class SosNativeCaller
    {
        using Armor = RE::TESObjectARMO;

    public:
        struct Awaitable
        {
            struct CallbackFunctor : public RE::BSScript::IStackCallbackFunctor
            {
                void operator()(RE::BSScript::Variable a_result) override;

                void SetObject([[maybe_unused]] const RE::BSTSmartPointer<RE::BSScript::Object> &a_object) override
                {
                }

                bool                    pending{false};
                RE::BSScript::Variable  result;
                std::coroutine_handle<> continuation;
            };

            Awaitable() : callback(new CallbackFunctor())
            {
            }

            void SetPending(bool a_pending = true) const
            {
                if (callback)
                {
                    dynamic_cast<CallbackFunctor *>(callback.get())->pending = a_pending;
                }
            }

            bool                   await_ready() const;
            void                   await_suspend(std::coroutine_handle<> a_handle) const;
            RE::BSScript::Variable await_resume() const;

            constexpr auto GetCallback() -> RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> &
            {
                return callback;
            }

        private:
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
        };

        static auto ActorNearPC() -> Awaitable;
        static auto AddActor(RE::Actor *actor) -> Awaitable;
        static auto RemoveActor(RE::Actor *actor) -> Awaitable;
        static auto ListActor() -> Awaitable;

        static auto GetCarriedArmor(RE::Actor *actor) -> Awaitable;
        static auto GetWornItems(RE::Actor *actor) -> Awaitable;

        static auto CreateOutfit(std::string &&outfitName) -> Awaitable;
        static auto ActiveOutfit(RE::Actor *actor, std::string &&outfitName) -> Awaitable;
        static auto GetSelectedOutfit(RE::Actor *actor) -> Awaitable;
        static auto RenameOutfit(std::string &&outfitName, std::string &&newName) -> Awaitable;
        static auto DeleteOutfit(std::string &&outfitName) -> Awaitable;
        static auto AddArmorToOutfit(std::string &&outfitName, Armor *armor) -> Awaitable;
        static auto RemoveArmorFromOutfit(std::string &&outfitName, Armor *armor) -> Awaitable;
        static auto RemoveConflictingArmorsFrom(Armor *armor, std::string &&outfitName) -> Awaitable;
        static auto IsOutfitExisting(std::string &&outfitName) -> Awaitable;
        static auto GetOutfitList(bool favoritesOnly = false) -> Awaitable;
        static auto OverwriteOutfit(std::string &&outfitName, std::vector<Armor *> &armors) -> Awaitable;
        static auto PrepOutfitBodySlotListing(std::string &&outfitName) -> Awaitable;
        static auto GetOutfitBodySlotListingArmorForms() -> Awaitable;
        static auto GetOutfitNameMaxLength() -> Awaitable;
        static auto BodySlotPolicyNamesForOutfit(std::string &&outfitName) -> Awaitable;
        static auto SetBodySlotPoliciesForOutfit(std::string &&outfitName, uint32_t slotPos, std::string &&policyCode) -> Awaitable;

        // auto-switch
        static auto IsActorAutoSwitchEnabled(RE::Actor *actor) -> Awaitable;
        static auto SetActorAutoSwitchEnabled(RE::Actor *actor, bool &enabled) -> Awaitable;
        static auto SetStateOutfit(RE::Actor *actor, uint32_t &&location, std::string&& outfitName) -> Awaitable;
        static auto GetStateOutfit(RE::Actor *actor, uint32_t &&location) -> Awaitable;

        // Enable SkyrimOutfitSystem?
        static auto Enable(bool &isEnable) -> Awaitable;
        static auto IsEnabled() -> Awaitable;
        static auto ImportSettings() -> Awaitable;
        static auto ExportSettings() -> Awaitable;

    private:
        static auto GetVM() -> RE::BSTSmartPointer<RE::BSScript::IVirtualMachine>
        {
            auto *skyrimVm = RE::SkyrimVM::GetSingleton();
            return skyrimVm != nullptr ? skyrimVm->impl : nullptr;
        }

        static auto DispatchStaticCall(const RE::BSFixedString &a_fnName, RE::BSScript::IFunctionArguments *a_args,
                                       RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> &a_result)
        {
            if (const auto &vm = GetVM(); vm != nullptr)
            {
                return vm->DispatchStaticCall(SOS_NATIVE_CLASS_NAME, a_fnName, a_args, a_result);
            }
            return false;
        }

        static auto StaticCall(const RE::BSFixedString          &a_fnName,
                               RE::BSScript::IFunctionArguments *a_args = RE::MakeFunctionArguments()) -> Awaitable
        {
            Awaitable  awaitable;
            const bool success = DispatchStaticCall(a_fnName, a_args, awaitable.GetCallback());
            awaitable.SetPending(success);
            return awaitable;
        }
    };
}