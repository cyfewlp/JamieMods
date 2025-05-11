#include "data/ArmorGenerator.h"
#include "common/config.h"
#include "common/log.h"
#include "util/utils.h"

#include <RE/B/BSContainer.h>
#include <RE/F/FormTypes.h>
#include <RE/I/InventoryEntryData.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESObjectARMO.h>
#include <functional>

namespace
LIBC_NAMESPACE_DECL
{
auto ArmorItemVisitor::Visit(RE::InventoryEntryData *a_entryData) -> RE::BSContainer::ForEachResult
{
    if (const auto form = a_entryData->object; form && form->formType == RE::FormType::Armor)
    {
        if (auto armor = form->As<RE::TESObjectARMO>(); armor != nullptr)
        {
            armors.push_back(armor);
        }
    }
    return RE::BSContainer::ForEachResult::kContinue;
}

void FormIdArmorGenerator::for_each(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (const auto foundArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(armorFormId);
        foundArmor != nullptr)
    {
        action(foundArmor);
    }
}

void NearObjectsInventoryArmorGenerator::for_each(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (wantVisitIndex >= nearObjects.size())
    {
        return;
    }
    for (const auto &entry : nearObjects[wantVisitIndex]->GetInventory())
    {
        if (const auto armor = skyrim_cast<RE::TESObjectARMO *>(entry.first);
            armor != nullptr)
        {
            action(armor);
        }
    }
}

void NearObjectsInventoryArmorGenerator::Update()
{
    nearObjects.clear();
    const RE::PlayerCharacter *player = RE::PlayerCharacter::GetSingleton();
    const RE::TESObjectCELL *  cell   = player->GetParentCell();

    cell->ForEachReference([this](RE::TESObjectREFR *objectRef) {
        if (const auto &name = objectRef->GetName(); !name || !name[0])
        {
            return RE::BSContainer::ForEachResult::kContinue;
        }
        for (const auto &entry : objectRef->GetInventory())
        {
            if (const auto *armor = entry.first->As<RE::TESObjectARMO>();
                armor && util::IsArmorCanDisplay(armor))
            {
                nearObjects.push_back(objectRef);
                break;
            }
        }
        return RE::BSContainer::ForEachResult::kContinue;
    });
}

void InventoryArmorGenerator::for_each(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (actor == nullptr)
    {
        return;
    }

    ArmorItemVisitor visitor;
    actor->GetInventoryChanges()->VisitInventory(visitor);

    for (const auto &armor : visitor.armors)
    {
        if (util::IsArmorCanDisplay(armor))
        {
            action(armor);
        }
    }
}

void CarriedArmorGenerator::for_each(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    if (actor == nullptr)
    {
        return;
    }

    ArmorItemVisitor visitor;
    actor->GetInventoryChanges()->VisitWornItems(visitor);

    for (const auto &armor : visitor.armors)
    {
        if (util::IsArmorCanDisplay(armor))
        {
            action(armor);
        }
    }
}

void BasicArmorGenerator::for_each(std::function<void(RE::TESObjectARMO *armor)> &&action)
{
    auto *      dataHandler = RE::TESDataHandler::GetSingleton();
    const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();

    for (const auto &armor : armorArray)
    {
        if (util::IsArmorCanDisplay(armor))
        {
            action(armor);
        }
    }
}
}