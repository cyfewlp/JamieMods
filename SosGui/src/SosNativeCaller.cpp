#include "SosNativeCaller.h"

#include "SosDataType.h"
#include "common/config.h"

#include <RE/F/FunctionArguments.h>
#include <RE/V/Variable.h>
#include <coroutine>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    void SosNativeCaller::Awaitable::CallbackFunctor::operator()(RE::BSScript::Variable a_result)
    {
        pending = false;
        result  = a_result;

        if (continuation) { continuation.resume(); }
    }

    bool SosNativeCaller::Awaitable::await_ready() const
    {
        if (!callback) return true;

        return static_cast<CallbackFunctor *>(callback.get())->pending == false;
    }

    void SosNativeCaller::Awaitable::await_suspend(std::coroutine_handle<> a_handle) const
    {
        if (!callback)
        {
            a_handle.resume();
            return;
        }

        auto &continuation = static_cast<CallbackFunctor *>(callback.get())->continuation;

        if (continuation) { continuation.destroy(); }

        continuation = a_handle;
    }

    RE::BSScript::Variable SosNativeCaller::Awaitable::await_resume() const
    {
        return dynamic_cast<CallbackFunctor *>(callback.get())->result;
    }

    auto SosNativeCaller::GetOutfitNameMaxLength() -> Awaitable
    {
        return StaticCall(SosFunction::GetOutfitNameMaxLength);
    }

    auto SosNativeCaller::BodySlotPolicyNamesForOutfit(std::string &&outfitName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName));
        return StaticCall(SosFunction::BodySlotPolicyNamesForOutfit, args);
    }

    auto SosNativeCaller::SetBodySlotPoliciesForOutfit(std::string &&outfitName, uint32_t slotPos,
                                                       std::string &&policyCode) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(outfitName), std::move(slotPos), std::move(policyCode));
        return StaticCall(SosFunction::SetBodySlotPoliciesForOutfit, args);
    }

    auto SosNativeCaller::ActiveOutfit(RE::Actor *actor, std::string &&outfitName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor), std::forward<std::string>(outfitName));
        return StaticCall(SosFunction::SetSelectedOutfit, args);
    }

    auto SosNativeCaller::RenameOutfit(std::string &&outfitName, std::string &&newName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName), //
                                               std::forward<std::string>(newName));
        return StaticCall(SosFunction::RenameOutfit, args);
    }

    auto SosNativeCaller::DeleteOutfit(std::string &&outfitName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName));
        return StaticCall(SosFunction::DeleteOutfit, args);
    }

    auto SosNativeCaller::AddArmorToOutfit(std::string &&outfitName, Armor *armor) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName), std::move(armor));
        return StaticCall(SosFunction::AddArmorToOutfit, args);
    }

    auto SosNativeCaller::RemoveArmorFromOutfit(std::string &&outfitName, Armor *armor) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName), std::move(armor));
        return StaticCall(SosFunction::RemoveArmorFromOutfit, args);
    }

    auto SosNativeCaller::IsOutfitExisting(std::string &&outfitName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName));
        return StaticCall(SosFunction::OutfitExists, args);
    }

    auto SosNativeCaller::ActorNearPC() -> Awaitable { return StaticCall(SosFunction::ActorNearPC); }

    auto SosNativeCaller::AddActor(RE::Actor *actor) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor));
        return StaticCall(SosFunction::AddActor, args);
    }

    auto SosNativeCaller::RemoveActor(RE::Actor *actor) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor));
        return StaticCall(SosFunction::RemoveActor, args);
    }

    auto SosNativeCaller::ListActor() -> Awaitable { return StaticCall(SosFunction::ListActors); }

    auto SosNativeCaller::GetOutfitList(bool favoritesOnly) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(favoritesOnly));
        return StaticCall(SosFunction::ListOutfits, args);
    }

    auto SosNativeCaller::OverwriteOutfit(std::string &&outfitName, std::vector<RE::TESObjectARMO *> &armors)
        -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName), std::move(armors));
        return StaticCall(SosFunction::OverwriteOutfit, args);
    }

    auto SosNativeCaller::PrepOutfitBodySlotListing(std::string &&outfitName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName));
        return StaticCall(SosFunction::PrepOutfitBodySlotListing, args);
    }

    auto SosNativeCaller::GetOutfitBodySlotListingArmorForms() -> Awaitable
    {
        return StaticCall(SosFunction::GetOutfitBodySlotListingArmorForms);
    }

    auto SosNativeCaller::IsActorAutoSwitchEnabled(RE::Actor *actor) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor));
        return StaticCall(SosFunction::GetLocationBasedAutoSwitchEnabled, args);
    }

    auto SosNativeCaller::SetActorAutoSwitchEnabled(RE::Actor *actor, bool &enabled) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor), std::move(enabled));
        return StaticCall(SosFunction::SetLocationBasedAutoSwitchEnabled, args);
    }

    auto SosNativeCaller::SetStateOutfit(RE::Actor *actor, uint32_t &&location, std::string &&outfitName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor), std::move(location), std::move(outfitName));
        return StaticCall(SosFunction::SetStateOutfit, args);
    }

    auto SosNativeCaller::GetStateOutfit(RE::Actor *actor, uint32_t &&location) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor), std::move(location));
        return StaticCall(SosFunction::GetStateOutfit, args);
    }

    auto SosNativeCaller::GetCarriedArmor(RE::Actor *actor) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor));
        return StaticCall(SosFunction::GetCarriedArmor, args);
    }

    auto SosNativeCaller::GetWornItems(RE::Actor *actor) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(actor));
        return StaticCall(SosFunction::GetWornItems, args);
    }

    auto SosNativeCaller::CreateOutfit(std::string &&outfitName) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::forward<std::string>(outfitName));
        return StaticCall(SosFunction::CreateOutfit, args);
    }

    auto SosNativeCaller::ImportSettings() -> Awaitable { return StaticCall(SosFunction::ImportSettings); }

    auto SosNativeCaller::ExportSettings() -> Awaitable { return StaticCall(SosFunction::ExportSettings); }

    auto SosNativeCaller::Enable(bool &isEnable) -> Awaitable
    {
        auto *args = RE::MakeFunctionArguments(std::move(isEnable));
        return StaticCall(SosFunction::SetEnabled, args);
    }

    auto SosNativeCaller::IsEnabled() -> Awaitable { return StaticCall(SosFunction::IsEnabled); }
}