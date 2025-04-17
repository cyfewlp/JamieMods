#include "gui/SosDataCoordinator.h"
#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "SosUiData.h"
#include "common/config.h"
#include "common/log.h"
#include "coroutine.h"
#include "gui/OutfitEditPanel.h"

#include <RE/A/Actor.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/S/SpellItem.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/V/Variable.h>
#include <cstdint>
#include <string>
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

    auto SosDataCoordinator::RequestCreateOutfit(std::string outfitName) const -> CoroutineTask
    {
        Variable isAlreadyExist = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
        if (isAlreadyExist.IsBool() && isAlreadyExist.GetBool())
        {
            co_return;
        }
        co_await SosNativeCaller::CreateOutfit(std::string(outfitName));
        Variable existVar = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
        if (!existVar.IsBool() || !existVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
        }
        else
        {
            co_await m_uiData.await_execute_on_ui();
            m_uiData.GetOutfitList().AddOutfit(std::move(outfitName));
        }
    }

    auto SosDataCoordinator::RequestCreateOutfitFromWorn(std::string outfitName) const -> CoroutineTask
    {
        std::string errorMessage("Can't create outfit from worn: ");
        if (auto *player = RE::PlayerCharacter::GetSingleton(); player != nullptr)
        {
            Variable wornArmorsVar = co_await SosNativeCaller::GetWornItems(player);
            if (wornArmorsVar.IsObjectArray())
            {
                auto                             array = wornArmorsVar.GetArray();
                std::vector<RE::TESObjectARMO *> armors;
                for (auto *iter = array->begin(); iter != array->end(); ++iter)
                {
                    const RE::BSScript::Variable var = *iter;
                    armors.emplace_back(var.Unpack<RE::TESObjectARMO *>());
                }
                co_await SosNativeCaller::OverwriteOutfit(std::string(outfitName), armors);
                Variable existVar = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
                if (existVar.IsBool() && existVar.GetBool())
                {
                    co_await m_uiData.await_execute_on_ui();
                    m_uiData.GetOutfitList().AddOutfit(std::move(outfitName));
                    co_return;
                }
                errorMessage.append("create outfit fail.");
            }
            else
            {
                errorMessage.append("can't get player worn armors.");
            }
        }
        else
        {
            errorMessage.append("can't get player");
        }

        m_uiData.PushErrorMessage(std::move(errorMessage));
    }

    auto SosDataCoordinator::RequestOutfitList() const -> CoroutinePromise
    {
        const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList();
        if (!outfitListVar.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
            co_return;
        }
        auto array = outfitListVar.GetArray();

        std::list<std::string> outfitNames;
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            outfitNames.emplace_back(var.Unpack<std::string>());
        }
        co_await m_uiData.await_execute_on_ui();
        auto &outfitList = m_uiData.GetOutfitList();
        outfitList.clear();
        outfitList.AddOutfits(std::move(outfitNames));
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

    auto SosDataCoordinator::RequestActiveOutfit(RE::Actor *actor, std::string outfitName) const -> CoroutineTask
    {
        co_await SosNativeCaller::ActiveOutfit(actor, std::string(outfitName));
        co_await m_uiData.await_execute_on_ui();
        m_uiData.SetActorActiveOutfit(actor, std::move(outfitName));
    }

    auto SosDataCoordinator::RequestRenameOutfit(SosUiData::OutfitPair pair, std::string newName) const -> CoroutineTask
    {
        Variable successVar =
            co_await SosNativeCaller::RenameOutfit(std::string(pair.second->GetName()), std::string(newName));
        if (!successVar.IsBool() || !successVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't rename outfit");
            co_return;
        }

        co_await m_uiData.await_execute_on_ui();
        m_uiData.GetOutfitList().RenameOutfit(pair.first, std::move(newName));
    }

    auto SosDataCoordinator::RequestDeleteOutfit(SosUiData::OutfitPair pair) const -> CoroutineTask
    {
        co_await SosNativeCaller::DeleteOutfit(std::string(pair.second->GetName()));
        co_await m_uiData.await_execute_on_ui();
        m_uiData.GetOutfitList().DeleteOutfit(pair.first);
    }

    auto SosDataCoordinator::RequestAddArmor(SosUiData::OutfitPair pair, Armor *armor) const -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::AddArmorToOutfit(std::string(pair.second->GetName()), armor);
        co_await m_uiData.await_execute_on_ui();
        m_uiData.GetOutfitList().AddArmor(pair.first, armor);
    }

    auto SosDataCoordinator::RequestDeleteConflictArmorsWith(SosUiData::OutfitPair pair, Armor *armor) const
        -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::RemoveConflictingArmorsFrom(armor, std::string(pair.second->GetName()));
        // We no need update UI data because we default directly override armor in slot
    }

    auto SosDataCoordinator::RequestDeleteArmor(SosUiData::OutfitPair pair, Armor *armor) const -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::RemoveArmorFromOutfit(std::string(pair.second->GetName()), armor);
        co_await m_uiData.await_execute_on_ui();
        m_uiData.GetOutfitList().DeleteArmor(pair.first, armor);
    }

    auto SosDataCoordinator::RequestOutfitArmors(SosUiData::OutfitPair pair) const -> CoroutineTask
    {
        co_await SosNativeCaller::PrepOutfitBodySlotListing(std::string(pair.second->GetName()));
        Variable armorsVar = co_await SosNativeCaller::GetOutfitBodySlotListingArmorForms();
        if (!armorsVar.IsObjectArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit armors");
            co_return;
        }
        auto                 array = armorsVar.GetArray();
        std::vector<Armor *> armors;
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            armors.emplace_back(var.Unpack<RE::TESObjectARMO *>());
        }
        co_await m_uiData.await_execute_on_ui();
        m_uiData.GetOutfitList().AddArmors(pair.first, std::move(armors));
    }

    auto SosDataCoordinator::RequestSetOutfitSlotPolicy(SosUiData::OutfitPair pair, uint32_t slotPos,
                                                        SlotPolicy policy) const -> CoroutineTask
    {
        co_await SosNativeCaller::SetBodySlotPoliciesForOutfit(std::string(pair.second->GetName()), slotPos,
                                                               SlotPolicyToCode(policy));
        co_await m_uiData.await_execute_on_ui();
        m_uiData.GetOutfitList().SetSlotPolicy(pair.first, slotPos, SlotPolicyToUiString(policy));
    }

    auto SosDataCoordinator::RequestOutfitSlotPolicy(SosUiData::OutfitPair pair) const -> CoroutineTask
    {
        Variable variable = co_await SosNativeCaller::BodySlotPolicyNamesForOutfit(std::string(pair.second->GetName()));
        if (!variable.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit slot policies");
            co_return;
        }
        auto array = variable.GetArray();
        if (array->size() < SosUiOutfit::SLOT_COUNT)
        {
            m_uiData.PushErrorMessage("Invalid outfit slot policies: slot count incorrect.");
            co_return;
        }
        std::vector<std::string> slotPolicies;
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;

            slotPolicies.emplace_back(var.GetString());
        }

        co_await m_uiData.await_execute_on_ui();

        m_uiData.GetOutfitList().SetAllSlotPolicies(pair.first, slotPolicies);
    }

    auto SosDataCoordinator::RequestGetArmorsByCarried() const -> CoroutineTask
    {
        std::string errorMessage;
        auto       *player = RE::PlayerCharacter::GetSingleton();
        if (player == nullptr)
        {
            errorMessage.append("can't get player");
        }
        else
        {
            Variable carriedArmorsVar = co_await SosNativeCaller::GetCarriedArmor(player);
            if (carriedArmorsVar.IsObjectArray())
            {
                auto                             array = carriedArmorsVar.GetArray();
                std::vector<RE::TESObjectARMO *> armors;
                for (auto *iter = array->begin(); iter != array->end(); ++iter)
                {
                    const RE::BSScript::Variable var = *iter;
                    armors.emplace_back(var.Unpack<RE::TESObjectARMO *>());
                }
                co_await m_uiData.await_execute_on_ui();
                m_uiData.SetArmorCandidates(std::move(armors));
            }
            else
            {
                errorMessage.append("can't get player carried armors");
            }
        }
    }

    auto SosDataCoordinator::RequestGetArmorsByWorn() const -> CoroutineTask
    {
        std::string errorMessage;
        auto       *player = RE::PlayerCharacter::GetSingleton();
        if (player == nullptr)
        {
            errorMessage.append("can't get player");
        }
        else
        {
            Variable wornArmorsVar = co_await SosNativeCaller::GetWornItems(player);
            if (wornArmorsVar.IsObjectArray())
            {
                auto                             array = wornArmorsVar.GetArray();
                std::vector<RE::TESObjectARMO *> armors;
                for (auto *iter = array->begin(); iter != array->end(); ++iter)
                {
                    const RE::BSScript::Variable var = *iter;
                    armors.emplace_back(var.Unpack<RE::TESObjectARMO *>());
                }
                co_await m_uiData.await_execute_on_ui();
                m_uiData.SetArmorCandidates(std::move(armors));
            }
            else
            {
                errorMessage.append("can't get player worn armors");
            }
        }
    }

    auto SosDataCoordinator::RequestActorStateOutfit(RE::Actor *actor, StateType location) const -> CoroutineTask
    {
        Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, static_cast<uint32_t>(location));
        if (!outfitVar.IsString())
        {
            m_uiData.PushErrorMessage("Can't get actor state outfit");
            co_return;
        }
        co_await m_uiData.await_execute_on_ui();
        m_uiData.PutActorOutfitState(actor, std::make_pair(location, std::string(outfitVar.GetString())));
    }

    auto SosDataCoordinator::RequestSetActorStateOutfit(RE::Actor *actor, StateType location,
                                                        std::string outfitName) const -> CoroutineTask
    {
        co_await SosNativeCaller::SetStateOutfit(actor, static_cast<uint32_t>(location), std::string(outfitName));
        m_uiData.PutActorOutfitState(actor, std::make_pair(location, std::move(outfitName)));
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

    auto SosDataCoordinator::Refresh() const -> CoroutineTask
    {
        auto task1 = RequestActorList();
        auto task2 = RequestOutfitList();
        auto task3 = RequestUpdateActorAutoSwitchState(RE::PlayerCharacter::GetSingleton());
        auto task4 = QueryIsEnable();
        m_uiData.SetQuickSlotEnabled(HasQuickSlotSpell());

        co_await task1;
        co_await task2;
        co_await task3;
        co_await task4;
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