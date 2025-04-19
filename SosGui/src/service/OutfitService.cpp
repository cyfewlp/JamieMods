#include "service/OutfitService.h"
#include "SosNativeCaller.h"
#include "data/SosUiData.h"

#include <string>

namespace LIBC_NAMESPACE_DECL
{
    auto OutfitService::CreateOutfit(std::string outfitName) const -> CoroutineTask
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
            m_outfitList.AddOutfit(std::move(outfitName));
        }
    }

    auto OutfitService::CreateOutfitFromWorn(std::string outfitName) const -> CoroutineTask
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

    auto OutfitService::GetOutfitList() const -> CoroutinePromise
    {
        const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList();
        if (!outfitListVar.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
            co_return;
        }
        auto array = outfitListVar.GetArray();

        m_outfitList.clear();
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const RE::BSScript::Variable var = *iter;
            //outfitNames.emplace_back(var.Unpack<std::string>());
            m_outfitList.AddOutfit(var.Unpack<std::string>());
        }
        //co_await m_uiData.await_execute_on_ui();
    }

    auto OutfitService::RequestFavoriteOutfits() const -> CoroutinePromise
    {
        const RE::BSScript::Variable outfitListVar = co_await SosNativeCaller::GetOutfitList(true);
        if (!outfitListVar.IsLiteralArray())
        {
            m_uiData.PushErrorMessage("Can't get outfit list");
            co_return;
        }
        auto array = outfitListVar.GetArray();

        std::list<std::string> outfitNames;
        for (auto *iter = array->begin(); iter != array->end(); ++iter)
        {
            const auto id = m_outfitList.findByName(iter->Unpack<std::string>());
            m_outfitList.SetFavoriteOutfit(id, true);
        }
    }

    auto OutfitService::SetActorOutfit(RE::Actor *actor, OutfitId id, std::string outfitName) const -> CoroutineTask
    {
        co_await SosNativeCaller::ActiveOutfit(actor, std::string(outfitName));
        co_await m_uiData.await_execute_on_ui();
        m_uiData.GetActorOutfitMap().SetOutfit(actor, id);
    }

    auto OutfitService::GetActorOutfit(RE::Actor *actor) const -> CoroutineTask
    {
        Variable outfitNameVar = co_await SosNativeCaller::GetSelectedOutfit(actor);
        if (!outfitNameVar.IsString())
        {
            m_uiData.PushErrorMessage("Can't get actor's active outfit");
        }
        co_await m_uiData.await_execute_on_ui();
        const auto outfitName = outfitNameVar.Unpack<std::string>();
        if (const auto id = m_outfitList.findByName(outfitNameVar.Unpack<std::string>()); id != INVALID_ID)
        {
            m_uiData.GetActorOutfitMap().SetOutfit(actor, id);
        }
    }

    auto OutfitService::RenameOutfit(OutfitId id, std::string outfitName, std::string newName) const -> CoroutineTask
    {
        Variable successVar = co_await SosNativeCaller::RenameOutfit(std::move(outfitName), std::string(newName));
        if (!successVar.IsBool() || !successVar.GetBool())
        {
            m_uiData.PushErrorMessage("Can't rename outfit");
            co_return;
        }

        co_await m_uiData.await_execute_on_ui();
        m_outfitList.RenameOutfit(id, std::move(newName));
    }

    auto OutfitService::DeleteOutfit(OutfitId id, std::string outfitName) const -> CoroutineTask
    {
        co_await SosNativeCaller::DeleteOutfit(std::move(outfitName));
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.DeleteOutfit(id);
    }

    auto OutfitService::AddArmor(OutfitId id, std::string outfitName, Armor *armor) const -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::AddArmorToOutfit(std::move(outfitName), armor);
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.AddArmor(id, armor);
    }

    auto OutfitService::DeleteConflictArmors(std::string outfitName, Armor *armor) const -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::RemoveConflictingArmorsFrom(armor, std::move(outfitName));
        // We no need update UI data because we default directly override armor in slot
    }

    auto OutfitService::DeleteArmor(OutfitId id, std::string outfitName, Armor *armor) const -> CoroutineTask
    {
        if (armor == nullptr)
        {
            co_return;
        }
        co_await SosNativeCaller::RemoveArmorFromOutfit(std::move(outfitName), armor);
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.DeleteArmor(id, armor);
    }

    auto OutfitService::GetOutfitArmors(OutfitId id, std::string outfitName) const -> CoroutineTask
    {
        co_await SosNativeCaller::PrepOutfitBodySlotListing(std::move(outfitName));
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
        m_outfitList.AddArmors(id, std::move(armors));
    }

    auto OutfitService::SetSlotPolicy(OutfitId id, std::string outfitName, uint32_t slotPos, SlotPolicy policy) const
        -> CoroutineTask
    {
        co_await SosNativeCaller::SetBodySlotPoliciesForOutfit(std::move(outfitName), slotPos,
                                                               SlotPolicyToCode(policy));
        co_await m_uiData.await_execute_on_ui();
        m_outfitList.SetSlotPolicy(id, slotPos, SlotPolicyToUiString(policy));
    }

    auto OutfitService::GetSlotPolicy(OutfitId id, std::string outfitName) const -> CoroutineTask
    {
        Variable variable = co_await SosNativeCaller::BodySlotPolicyNamesForOutfit(std::move(outfitName));
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

        m_outfitList.SetAllSlotPolicies(id, slotPolicies);
    }

    auto OutfitService::GetActorStateOutfit(RE::Actor *actor, StateType location) const -> CoroutineTask
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

    auto OutfitService::SetActorStateOutfit(RE::Actor *actor, StateType location, std::string outfitName) const
        -> CoroutineTask
    {
        co_await SosNativeCaller::SetStateOutfit(actor, static_cast<uint32_t>(location), std::string(outfitName));
        m_uiData.PutActorOutfitState(actor, std::make_pair(location, std::move(outfitName)));
    }

    auto OutfitService::GetArmorsByCarried() const -> CoroutineTask
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

    auto OutfitService::GetArmorsByWorn() const -> CoroutineTask
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
}