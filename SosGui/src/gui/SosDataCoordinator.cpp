#include "gui/SosDataCoordinator.h"
#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "SosOutfit.h"
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

            auto outfit = SosOutfit(var.GetString().data());
            outfitMap.emplace(var.GetString(), outfit);
        }
        onComplete();
    }

    auto SosDataCoordinator::RequestUpdateActorAutoSwitchState(RE::Actor *actor, OnComplete onComplete) -> CoroutineTask
    {
        RE::BSScript::Variable isEnabledVar = co_await SosNativeCaller::IsActorAutoSwitchEnabled(actor);
        if (!isEnabledVar.IsBool())
        {
            m_uiData.PushErrorMessage("Can't get actor auto-switch enabled state");
            co_return;
        }
        auto isEnabled = isEnabledVar.GetBool();
        m_uiData.SetAutoSwitchEnabled(actor, isEnabled);
        onComplete();
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
