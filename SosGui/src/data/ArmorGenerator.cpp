#include "data/ArmorGenerator.h"
#include "common/config.h"

#include <RE/B/BSContainer.h>
#include <RE/F/FormTypes.h>
#include <RE/I/InventoryEntryData.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESObjectARMO.h>
#include <functional>

namespace LIBC_NAMESPACE_DECL
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

    auto ArmorGenerator::IsArmorCanDisplay(RE::TESObjectARMO *armor) -> bool
    {
        bool canDisplay = false;
        if (armor != nullptr && armor->templateArmor == nullptr)
        {
            if (std::string_view name = armor->GetName(); !name.empty())
            {
                canDisplay = true;
            }
        }

        return canDisplay;
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
            if (IsArmorCanDisplay(armor))
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
            if (IsArmorCanDisplay(armor))
            {
                action(armor);
            }
        }
    }

    void BasicArmorGenerator::for_each(std::function<void(RE::TESObjectARMO *armor)> &&action)
    {
        auto       *dataHandler = RE::TESDataHandler::GetSingleton();
        const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();

        for (const auto &armor : armorArray)
        {
            if (IsArmorCanDisplay(armor))
            {
                action(armor);
            }
        }
    }
}