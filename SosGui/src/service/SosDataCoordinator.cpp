#include "service/SosDataCoordinator.h"

#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "common/config.h"
#include "coroutine.h"
#include "data/SosUiData.h"
#include "service/OutfitService.h"

#include <RE/A/Actor.h>
#include <RE/P/PackUnpack.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/S/SpellItem.h>
#include <RE/T/TESForm.h>
#include <RE/V/Variable.h>
#include <queue>
#include <type_traits>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    auto SosDataCoordinator::RequestActorList() const -> CoroutinePromise
    {
        const RE::BSScript::Variable actorListVar = co_await SosNativeCaller::ListActor();
        if (!actorListVar.IsObjectArray())
        {
            m_uiData.PushErrorMessage("Can't get actor list");
            co_return;
        }
        co_await m_uiData.await_execute_on_ui();
        auto  array      = actorListVar.GetArray();
        auto &actorsList = m_uiData.GetActors();
        actorsList.clear();
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            actorsList.emplace_back(var.Unpack<RE::Actor *>());
        }
    }

    auto SosDataCoordinator::RequestAddActor(RE::Actor *actor) const -> CoroutineTask
    {
        co_await SosNativeCaller::AddActor(actor);
        co_await m_uiData.await_execute_on_ui();
        m_uiData.AddActor(actor);
    }

    auto SosDataCoordinator::RequestRemoveActor(RE::Actor *actor) const -> CoroutineTask
    {
        co_await SosNativeCaller::RemoveActor(actor);
        co_await m_uiData.await_execute_on_ui();
        m_uiData.RemoveActor(actor);
    }

    auto SosDataCoordinator::RequestNearActorList() const -> CoroutineTask
    {
        Variable actorsVar = co_await SosNativeCaller::ActorNearPC();
        if (!actorsVar.IsObjectArray())
        {
            m_uiData.PushErrorMessage("Can't get near actor list");
            co_return;
        }
        co_await m_uiData.await_execute_on_ui();
        auto                     array = actorsVar.GetArray();
        std::vector<RE::Actor *> actorsList;
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            actorsList.emplace_back(var.Unpack<RE::Actor *>());
        }
        m_uiData.SetNearActors(actorsList);
    }

    auto SosDataCoordinator::RequestUpdateActorAutoSwitchState(RE::Actor *actor) const -> CoroutinePromise
    {
        RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsActorAutoSwitchEnabled(actor);
        if (!isEnabledVar.IsBool())
        {
            m_uiData.PushErrorMessage("Can't get actor auto-switch enabled state");
            co_return;
        }
        co_await m_uiData.await_execute_on_ui();
        auto isEnabled = isEnabledVar.GetBool();
        m_uiData.SetAutoSwitchEnabled(actor, isEnabled);
    }

    auto SosDataCoordinator::RequestSetActorAutoSwitchState(RE::Actor *actor, bool enabled) const -> CoroutineTask
    {
        co_await SosNativeCaller::SetActorAutoSwitchEnabled(actor, enabled);
        co_await m_uiData.await_execute_on_ui();
        m_uiData.SetAutoSwitchEnabled(actor, enabled);
    }

    auto SosDataCoordinator::RequestImportSettings() -> CoroutineTask
    {
        RE::BSScript::Variable successVar = co_await SosNativeCaller::ImportSettings();
        if (!successVar.IsBool() || !successVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't import settings");
            co_return;
        }
        co_await Refresh();
    }

    auto SosDataCoordinator::RequestExportSettings() const -> CoroutineTask
    {
        RE::BSScript::Variable successVar = co_await SosNativeCaller::ExportSettings();
        if (!successVar.IsBool() || !successVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't export settings");
            co_return;
        }
    }

    auto SosDataCoordinator::RequestEnable(bool isEnabled) const -> CoroutineTask
    {
        co_await SosNativeCaller::Enable(isEnabled);
        RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsEnabled();
        if (isEnabledVar.IsBool() && isEnabledVar.GetBool() != isEnabled)
        {
            m_uiData.PushErrorMessage("Can't set SkyrimOutfitSystem enabled state");
        }
        else
        {
            co_await m_uiData.await_execute_on_ui();
            m_uiData.SetEnabled(isEnabled);
        }
    }

    auto SosDataCoordinator::QueryIsEnable() const -> CoroutinePromise
    {
        RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsEnabled();
        if (!isEnabledVar.IsBool())
        {
            m_uiData.PushErrorMessage("Can't set SkyrimOutfitSystem enabled state");
            co_return;
        }
        co_await m_uiData.await_execute_on_ui();
        m_uiData.SetEnabled(isEnabledVar.GetBool());
    }

    static void operator+=(std::queue<CoroutinePromise> &promiseQueue, CoroutinePromise &&promise)
    {
        promiseQueue.emplace(std::forward<CoroutinePromise>(promise));
    }

    auto SosDataCoordinator::Refresh() const -> CoroutineTask
    {
        auto toPromise = [](auto &&task) -> CoroutinePromise {
            co_await task;
        };
        std::queue<CoroutinePromise> promiseQueue;
        promiseQueue += RequestActorList();
        promiseQueue += m_outfitService.GetOutfitList();
        promiseQueue += QueryIsEnable();
        promiseQueue += RequestUpdateActorAutoSwitchState(RE::PlayerCharacter::GetSingleton());
        promiseQueue += toPromise(m_outfitService.GetActorOutfit(RE::PlayerCharacter::GetSingleton()));

        m_uiData.SetQuickSlotEnabled(HasQuickSlotSpell());

        while (!promiseQueue.empty())
        {
            co_await promiseQueue.front();
            promiseQueue.pop();
        }
    }

    auto SosDataCoordinator::HasQuickSlotSpell() -> bool
    {
        const auto &player = RE::PlayerCharacter::GetSingleton();
        auto       *spell  = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID);
        if (spell != nullptr)
        {
            return player->HasSpell(spell);
        }
        return false;
    }
}