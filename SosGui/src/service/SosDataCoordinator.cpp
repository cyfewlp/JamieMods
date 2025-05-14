#include "service/SosDataCoordinator.h"

#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "common/config.h"
#include "data/SosUiData.h"
#include "service/OutfitService.h"

#include <RE/A/Actor.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/S/SpellItem.h>
#include <RE/T/TESForm.h>
#include <RE/V/Variable.h>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
auto SosDataCoordinator::RequestActorList() const -> Task
{
    const RE::BSScript::Variable actorListVar = co_await SosNativeCaller::ListActor();
    if (!actorListVar.IsObjectArray())
    {
        m_uiData.PushErrorMessage("Can't get actor list");
        co_return;
    }

    const auto array      = actorListVar.GetArray();
    auto      &actorsList = m_uiData.GetActors();
    actorsList.clear();
    for (const auto *iter = array->begin(); iter != array->end(); ++iter)
    {
        const RE::BSScript::Variable var   = *iter;
        auto                        *actor = var.Unpack<RE::Actor *>();
        actorsList.emplace_back(var.Unpack<RE::Actor *>());

        const RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsActorAutoSwitchEnabled(actor);
        if (isEnabledVar.IsBool() && isEnabledVar.GetBool())
        {
            co_await m_outfitService.GetActorAllStateOutfit(actor);
        }
        co_await m_outfitService.GetActorOutfit(actor);
        co_await RequestUpdateActorAutoSwitchState(actor);
    }
}

auto SosDataCoordinator::RequestAddActor(RE::Actor *actor) const -> Task
{
    co_await SosNativeCaller::AddActor(actor);
    m_uiData.AddActor(actor);
}

auto SosDataCoordinator::RequestRemoveActor(RE::Actor *actor) const -> Task
{
    co_await SosNativeCaller::RemoveActor(actor);
    m_uiData.RemoveActor(actor);
}

auto SosDataCoordinator::RequestNearActorList() const -> Task
{
    const Variable actorsVar = co_await SosNativeCaller::ActorNearPC();
    if (!actorsVar.IsObjectArray())
    {
        m_uiData.PushErrorMessage("Can't get near actor list");
        co_return;
    }

    const auto               array = actorsVar.GetArray();
    std::vector<RE::Actor *> actorsList;
    for (const auto *iter = array->begin(); iter != array->end(); ++iter)
    {
        const RE::BSScript::Variable var = *iter;
        actorsList.emplace_back(var.Unpack<RE::Actor *>());
    }
    m_uiData.SetNearActors(actorsList);
}

auto SosDataCoordinator::RequestUpdateActorAutoSwitchState(RE::Actor *actor) const -> Task
{
    const RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsActorAutoSwitchEnabled(actor);
    if (!isEnabledVar.IsBool())
    {
        m_uiData.PushErrorMessage("Can't get actor auto-switch enabled state");
        co_return;
    }
    const auto isEnabled = isEnabledVar.GetBool();
    m_uiData.SetAutoSwitchEnabled(actor->GetFormID(), isEnabled);
}

auto SosDataCoordinator::RequestSetActorAutoSwitchState(const RE::Actor *actor, bool enabled) const -> Task
{
    co_await SosNativeCaller::SetActorAutoSwitchEnabled(actor, enabled);
    m_uiData.SetAutoSwitchEnabled(actor->GetFormID(), enabled);
}

auto SosDataCoordinator::RequestImportSettings() const -> Task
{
    if (const auto successVar = co_await SosNativeCaller::ImportSettings();
        !successVar.IsBool() || !successVar.GetBool())
    {
        m_uiData.PushErrorMessage("Can't import settings");
        co_return;
    }
    co_await Refresh();
}

auto SosDataCoordinator::RequestExportSettings() const -> Task
{
    if (const RE::BSScript::Variable successVar = co_await SosNativeCaller::ExportSettings();
        !successVar.IsBool() || !successVar.GetBool())
    {
        m_uiData.PushErrorMessage("Can't export settings");
        co_return;
    }
}

auto SosDataCoordinator::RequestEnable(bool isEnabled) const -> Task
{
    co_await SosNativeCaller::Enable(isEnabled);
    if (RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsEnabled();
        isEnabledVar.IsBool() && isEnabledVar.GetBool() != isEnabled)
    {
        m_uiData.PushErrorMessage("Can't set SkyrimOutfitSystem enabled state");
    }
    else
    {
        m_uiData.SetEnabled(isEnabled);
    }
}

auto SosDataCoordinator::QueryIsEnable() const -> Task
{
    const RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsEnabled();
    if (!isEnabledVar.IsBool())
    {
        m_uiData.PushErrorMessage("Can't set SkyrimOutfitSystem enabled state");
        co_return;
    }
    m_uiData.SetEnabled(isEnabledVar.GetBool());
}

auto SosDataCoordinator::Refresh() const -> Task
{
    log_debug("start refresh in thread: {}", std::this_thread::get_id());
    auto start = std::chrono::high_resolution_clock::now();
    co_await m_outfitService.GetOutfitList();
    co_await QueryIsEnable();
    co_await RequestActorList();
    co_await RequestUpdateActorAutoSwitchState(RE::PlayerCharacter::GetSingleton());
    co_await m_outfitService.GetActorOutfit(RE::PlayerCharacter::GetSingleton());
    co_await RequestNearActorList();
    co_await m_outfitService.GetAllFavoriteOutfits();
    m_uiData.SetQuickSlotEnabled(HasQuickSlotSpell());
    auto end = std::chrono::high_resolution_clock::now();

    auto nano = std::chrono::nanoseconds(end - start);

    log_info("refresh spent: {}ns", nano.count());
}

auto SosDataCoordinator::HasQuickSlotSpell() -> bool
{
    const auto &player = RE::PlayerCharacter::GetSingleton();
    if (auto *spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID); spell != nullptr)
    {
        return player->HasSpell(spell);
    }
    return false;
}
}