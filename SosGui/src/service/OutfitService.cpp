#include "service/OutfitService.h"
#include "SosDataType.h"
#include "SosNativeCaller.h"
#include "common/config.h"
#include "data/AutoSwitchPolicyView.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "task.h"

#include <RE/A/Actor.h>
#include <RE/B/BipedObjects.h>
#include <RE/P/PackUnpack.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/V/Variable.h>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <cstdint>
#include <format>
#include <list>
#include <string>
#include <type_traits>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    auto OutfitService::CreateOutfit(std::string outfitName) const -> Task
    {
        if (const Variable isAlreadyExist = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
            isAlreadyExist.IsBool() && isAlreadyExist.GetBool())
        {
            co_return;
        }
        co_await SosNativeCaller::CreateOutfit(std::string(outfitName));
        if (const Variable existVar = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
            !existVar.IsBool() || !existVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
        }
        else
        {
            co_await m_uiData.await_execute_on_ui();
            m_outfitList.AddOutfit(std::move(outfitName));
        }
    }

    auto OutfitService::CreateOutfitFromWorn(std::string outfitName) const -> Task
    {
        std::string errorMessage("Can't create outfit from worn: ");
        if (auto *player = RE::PlayerCharacter::GetSingleton(); player != nullptr)
        {
            if (const Variable wornArmorsVar = co_await SosNativeCaller::GetWornItems(player);
                wornArmorsVar.IsObjectArray())
            {
                const auto                       array = wornArmorsVar.GetArray();
                std::vector<RE::TESObjectARMO *> armors;
                for (const auto *iter = array->begin(); iter != array->end(); ++iter)
                {
                    const RE::BSScript::Variable var = *iter;
                    armors.emplace_back(var.Unpack<RE::TESObjectARMO *>());
                }
                co_await SosNativeCaller::OverwriteOutfit(std::string(outfitName), armors);
                if (const Variable existVar = co_await SosNativeCaller::IsOutfitExisting(std::string(outfitName));
                    existVar.IsBool() && existVar.GetBool())
                {
                    co_await m_uiData.await_execute_on_ui();
                    m_outfitList.AddOutfit(std::move(outfitName));
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

    auto OutfitService::GetOutfitList() const -> Task
    {
        const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList();
        if (!outfitListVar.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
            co_return;
        }
        const auto array = outfitListVar.GetArray();

        m_outfitList.clear();
        for (const auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            // outfitNames.emplace_back(var.Unpack<std::string>());
            m_outfitList.AddOutfit(var.Unpack<std::string>());
        }
        // co_await m_uiData.await_execute_on_ui();
    }

    auto OutfitService::GetAllFavoriteOutfits() const -> Task
    {
        const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList(true);
        if (!outfitListVar.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
            co_return;
        }
        const auto array = outfitListVar.GetArray();

        auto &outfitList = m_uiData.GetOutfitList();
        for (const auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            if (const auto result = outfitList.SetFavoriteOutfit(iter->Unpack<std::string>(), true);
                !result.has_value())
            {
                m_uiData.PushErrorMessage(result.error().what());
                co_return;
            }
        }
    }

    auto OutfitService::SetOutfitIsFavorite(const OutfitId id, std::string outfitName, const bool isFavorite) const
        -> Task
    {
        if (const auto result = m_uiData.GetOutfitList().SetFavoriteOutfit(id, isFavorite); !result.has_value())
        {
            m_uiData.PushErrorMessage(result.error().what());
            co_return;
        }
        co_await SosNativeCaller::SetOutfitFavoriteStatus(std::string(outfitName), isFavorite);
    }

    auto OutfitService::SetActorOutfit(RE::Actor *actor, const OutfitId id, std::string outfitName) const -> Task
    {
        m_uiData.GetActorOutfitMap().SetOutfit(actor, id);
        co_await SosNativeCaller::ActiveOutfit(actor, std::string(outfitName));
    }

    auto OutfitService::GetActorOutfit(RE::Actor *actor) const -> Task
    {
        const Variable outfitNameVar = co_await SosNativeCaller::GetSelectedOutfit(actor);
        if (!outfitNameVar.IsString())
        {
            m_uiData.PushErrorMessage("Can't get actor's active outfit");
        }
        co_await m_uiData.await_execute_on_ui();
        const auto outfitName = outfitNameVar.Unpack<std::string>();
        if (const auto id = m_outfitList.findIdByName(outfitNameVar.Unpack<std::string>()); id != INVALID_OUTFIT_ID)
        {
            m_uiData.GetActorOutfitMap().SetOutfit(actor, id);
        }
    }

    auto OutfitService::RenameOutfit(const OutfitId id, std::string outfitName, std::string newName) const -> Task
    {
        if (const Variable successVar =
                co_await SosNativeCaller::RenameOutfit(std::move(outfitName), std::string(newName));
            !successVar.IsBool() || !successVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't rename outfit");
            co_return;
        }

        co_await m_uiData.await_execute_on_ui();
        m_outfitList.RenameOutfit(id, std::move(newName));
    }

    auto OutfitService::DeleteOutfit(const OutfitId id, std::string outfitName) const -> Task
    {
        co_await SosNativeCaller::DeleteOutfit(std::move(outfitName));
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.DeleteOutfit(id);
    }

    auto OutfitService::AddArmor(const OutfitId id, std::string outfitName, Armor *armor) const -> Task
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::AddArmorToOutfit(std::move(outfitName), armor);
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.AddArmor(id, armor);
    }

    auto OutfitService::DeleteConflictArmors(std::string outfitName, Armor *armor) -> Task
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::RemoveConflictingArmorsFrom(armor, std::move(outfitName));
        // We no need update UI data because we default directly override armor in slot
    }

    auto OutfitService::DeleteArmor(const OutfitId id, std::string outfitName, Armor *armor) const -> Task
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::RemoveArmorFromOutfit(std::move(outfitName), armor);
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.DeleteArmor(id, armor);
    }

    auto OutfitService::GetOutfitArmors(const OutfitId id, std::string outfitName) const -> Task
    {
        co_await SosNativeCaller::PrepOutfitBodySlotListing(std::move(outfitName));
        const Variable armorsVar = co_await SosNativeCaller::GetOutfitBodySlotListingArmorForms();
        if (!armorsVar.IsObjectArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit armors");
            co_return;
        }
        const auto           array = armorsVar.GetArray();
        std::vector<Armor *> armors;
        for (const auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            armors.emplace_back(var.Unpack<RE::TESObjectARMO *>());
        }
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.AddArmors(id, std::move(armors));
    }

    auto OutfitService::SetSlotPolicy(const OutfitId id, std::string outfitName, const uint32_t slotPos,
                                      const SlotPolicy policy) const -> Task
    {
        co_await SosNativeCaller::SetBodySlotPoliciesForOutfit(std::move(outfitName), slotPos,
                                                               SlotPolicyToCode(policy));
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.SetSlotPolicy(id, slotPos, SlotPolicyToUiString(policy));
    }

    auto OutfitService::GetSlotPolicy(const OutfitId id, std::string outfitName) const -> Task
    {
        const Variable variable = co_await SosNativeCaller::BodySlotPolicyNamesForOutfit(std::move(outfitName));
        if (!variable.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit slot policies");
            co_return;
        }
        const auto array = variable.GetArray();
        if (array->size() < SosUiOutfit::SLOT_COUNT)
        {
            m_uiData.PushErrorMessage("Invalid outfit slot policies: slot count incorrect.");
            co_return;
        }
        std::vector<std::string> slotPolicies;
        for (const auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;

            slotPolicies.emplace_back(var.GetString());
        }

        co_await m_uiData.await_execute_on_ui();

        m_outfitList.SetAllSlotPolicies(id, slotPolicies);
    }

    auto OutfitService::GetActorStateOutfit(RE::Actor *actor, uint32_t policyId) const -> Task
    {
        const Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, std::move(policyId));
        if (!outfitVar.IsString())
        {
            m_uiData.PushErrorMessage("Can't get actor state outfit");
            co_return;
        }

        if (const auto opt = m_uiData.GetOutfitList().TryFindIdByName(outfitVar.Unpack<std::string>()); opt.has_value())
        {
            m_uiData.GetAutoSwitchPolicyView().emplace(actor->GetFormID(), policyId, opt.value());
        }
    }

    auto OutfitService::GetActorAllStateOutfit(RE::Actor *actor) const -> Task
    {
        auto       &view       = m_uiData.GetAutoSwitchPolicyView();
        const auto &outfitList = m_uiData.GetOutfitList();
        view.erase(actor->GetFormID());
        for (uint32_t intState = 0; intState < RE::BIPED_OBJECT::kEditorTotal; ++intState)
        {
            Variable outfitVar = co_await SosNativeCaller::GetStateOutfit(actor, std::move(intState));
            if (!outfitVar.IsString())
            {
                continue;
            }
            if (const auto outfitId = outfitList.findIdByName(outfitVar.Unpack<std::string>());
                outfitId != INVALID_OUTFIT_ID)
            {
                view.emplace(actor->GetFormID(), intState, outfitId);
            }
        }
    }

    auto OutfitService::SetActorStateOutfit(RE::Actor *actor, uint32_t policyId, const OutfitId outfitId) const -> Task
    {
        if (policyId >= static_cast<uint32_t>(AutoSwitchPolicyView::Policy::Count))
        {
            m_uiData.PushErrorMessage(std::format("Invalid outfit policy: {}", policyId));
            co_return;
        }
        auto &view = m_uiData.GetAutoSwitchPolicyView();
        auto  outfitNameOpt =
            m_uiData.GetOutfitList().GetOutfitById(outfitId).map([](auto &outfit) { return outfit.GetName(); });
        const auto actorId = actor->GetFormID();
        if (!outfitNameOpt.has_value())
        {
            co_await SosNativeCaller::SetStateOutfit(actor, std::move(policyId), "");
            view.erase(actorId, policyId);
        }
        else
        {
            co_await SosNativeCaller::SetStateOutfit(actor, std::move(policyId), outfitNameOpt.value().c_str());
            view.emplace(actorId, policyId, outfitId);
        }
    }
}