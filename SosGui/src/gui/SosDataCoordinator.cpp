#include "gui/SosDataCoordinator.h"
#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "SosUiData.h"
#include "common/config.h"
#include "common/log.h"

#include <RE/A/Actor.h>
#include <RE/P/PackUnpack.h>
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
    auto SosDataCoordinator::RequestActorList(OnComplete onComplete) -> CoroutineTask
    {
        const RE::BSScript::Variable actorListVar = co_await SosNativeCaller::ListActor();
        if (!actorListVar.IsObjectArray())
        {
            m_uiData.PushErrorMessage("Can't get actor list");
            co_return;
        }
        auto  array      = actorListVar.GetArray();
        auto &actorsList = m_uiData.GetActors();
        actorsList.clear();
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            actorsList.push_back(var.Unpack<RE::Actor *>());
        }
        onComplete();
    }

    auto SosDataCoordinator::RequestAddActor(RE::Actor *actor) -> CoroutineTask
    {
        co_await SosNativeCaller::AddActor(actor);
        m_uiData.AddActor(actor);
    }

    auto SosDataCoordinator::RequestRemoveActor(RE::Actor *actor) -> CoroutineTask
    {
        co_await SosNativeCaller::RemoveActor(actor);
        m_uiData.RemoveActor(actor);
    }

    auto SosDataCoordinator::RequestNearActorList() -> CoroutineTask
    {
        Variable actorsVar = co_await SosNativeCaller::ActorNearPC();
        if (!actorsVar.IsObjectArray())
        {
            m_uiData.PushErrorMessage("Can't get near actor list");
            co_return;
        }
        auto                     array = actorsVar.GetArray();
        std::vector<RE::Actor *> actorsList;
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            actorsList.emplace_back(var.Unpack<RE::Actor *>());
        }
        m_uiData.SetNearActors(actorsList);
    }

    auto SosDataCoordinator::RequestCreateOutfit(std::string outfitName) -> CoroutineTask
    {
        co_await SosNativeCaller::CreateOutfit(std::string(outfitName));
        log_debug("now outfitName is {}", outfitName);
        Variable existVar = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
        if (!existVar.IsBool() || !existVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
        }
        else
        {
            m_uiData.AddOutfit(std::move(outfitName));
        }
    }

    auto SosDataCoordinator::RequestCreateOutfitFromWorn(std::string outfitName) -> CoroutineTask
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
                    m_uiData.AddOutfit(std::move(outfitName));
                    co_return;
                }
                else
                {
                    errorMessage.append("create outfit fail.");
                }
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

    auto SosDataCoordinator::RequestOutfitList(OnComplete onComplete) -> CoroutineTask
    {
        const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList();
        if (!outfitListVar.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
            co_return;
        }
        auto  array     = outfitListVar.GetArray();
        auto &outfitMap = m_uiData.GetOutfitMap();
        outfitMap.clear();
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            outfitMap.emplace(var.GetString(), std::string(var.GetString()));
        }
        onComplete();
    }

    auto SosDataCoordinator::RequestUpdateActorAutoSwitchState(RE::Actor *actor) -> CoroutineTask
    {
        RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsActorAutoSwitchEnabled(actor);
        if (!isEnabledVar.IsBool())
        {
            m_uiData.PushErrorMessage("Can't get actor auto-switch enabled state");
            co_return;
        }
        auto isEnabled = isEnabledVar.GetBool();
        m_uiData.SetAutoSwitchEnabled(actor, isEnabled);
    }

    auto SosDataCoordinator::RequestSetActorAutoSwitchState(RE::Actor *actor, bool enabled) -> CoroutineTask
    {
        co_await SosNativeCaller::SetActorAutoSwitchEnabled(actor, enabled);
        m_uiData.SetAutoSwitchEnabled(actor, enabled);
    }

    auto SosDataCoordinator::RequestRenameOutfit(std::string outfitName, std::string newName) -> CoroutineTask
    {
        Variable successVar = co_await SosNativeCaller::RenameOutfit(std::string(outfitName), std::string(newName));
        if (!successVar.IsBool() || !successVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't rename outfit");
            co_return;
        }
        m_uiData.RenameOutfit(std::move(outfitName), std::move(newName));
    }

    auto SosDataCoordinator::RequestActiveOutfit(RE::Actor *actor, std::string outfitName, OnComplete onComplete)
        -> CoroutineTask
    {
        co_await SosNativeCaller::ActiveOutfit(actor, std::string(outfitName));
        m_uiData.SetActorActiveOutfit(actor, std::move(outfitName));
        onComplete();
    }

    auto SosDataCoordinator::RequestDeleteOutfit(std::string outfitName, OnComplete onComplete) -> CoroutineTask
    {
        co_await SosNativeCaller::DeleteOutfit(std::string(outfitName));
        m_uiData.DeleteOutfit(std::move(outfitName));
        onComplete();
    }

    auto SosDataCoordinator::RequestAddArmor(std::string outfitName, Armor *armor) -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::AddArmorToOutfit(std::string(outfitName), armor);
        m_uiData.AddArmor(std::move(outfitName), armor);
    }

    auto SosDataCoordinator::RequestDeleteArmor(std::string outfitName, Armor *armor) -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::RemoveArmorFromOutfit(std::string(outfitName), armor);
        m_uiData.DeleteArmor(std::move(outfitName), armor);
    }

    auto SosDataCoordinator::RequestOutfitArmors(std::string outfitName) -> CoroutineTask
    {
        co_await SosNativeCaller::PrepOutfitBodySlotListing(std::string(outfitName));
        Variable armorsVar = co_await SosNativeCaller::GetOutfitBodySlotListingArmorForms();
        if (!armorsVar.IsObjectArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit armors");
            co_return;
        }
        auto  array      = armorsVar.GetArray();
        auto  outfitPair = m_uiData.GetOutfitMap().try_emplace(outfitName, outfitName);
        auto &outfit     = outfitPair.first->second;
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            outfit.AddArmor(var.Unpack<RE::TESObjectARMO *>());
        }
    }

    auto SosDataCoordinator::RequestGetArmorsByCarried() -> CoroutineTask
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
                m_uiData.SetArmorCandidates(std::move(armors));
            }
            else
            {
                errorMessage.append("can't get player carried armors");
            }
        }
    }

    auto SosDataCoordinator::RequestGetArmorsByWorn() -> CoroutineTask
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
                m_uiData.SetArmorCandidates(std::move(armors));
            }
            else
            {
                errorMessage.append("can't get player worn armors");
            }
        }
    }

    auto SosDataCoordinator::RequestActorStateOutfit(RE::Actor *actor, StateType location) -> CoroutineTask
    {
        Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, static_cast<uint32_t>(location));
        if (!outfitVar.IsString())
        {
            m_uiData.PushErrorMessage("Can't get actor state outfit");
            co_return;
        }
        m_uiData.PutActorOutfitState(actor, std::make_pair(location, std::string(outfitVar.GetString())));
    }

    auto SosDataCoordinator::RequestSetActorStateOutfit(RE::Actor *actor, StateType location, std::string outfitName)
        -> CoroutineTask
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
        Refresh();
    }

    auto SosDataCoordinator::RequestExportSettings() -> CoroutineTask
    {
        RE::BSScript::Variable successVar = co_await SosNativeCaller::ExportSettings();
        if (!successVar.IsBool() || !successVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't export settings");
            co_return;
        }
    }

    auto SosDataCoordinator::RequestEnable(bool isEnabled) -> CoroutineTask
    {
        co_await SosNativeCaller::Enable(isEnabled);
        RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsEnabled();
        if (isEnabledVar.IsBool() && isEnabledVar.GetBool() != isEnabled)
        {
            m_uiData.PushErrorMessage("Can't set SkyrimOutfitSystem enabled state");
        }
        else
        {
            m_uiData.SetEnabled(isEnabled);
        }
    }

    auto SosDataCoordinator::Refresh() -> CoroutineTask
    {
        RequestActorList();
        RequestOutfitList();
        RequestUpdateActorAutoSwitchState(RE::PlayerCharacter::GetSingleton());
        m_uiData.SetQuickSlotEnabled(HasQuickSlotSpell());
        return CoroutineTask();
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
