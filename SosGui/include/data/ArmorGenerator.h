#pragma once

#include "common/config.h"

#include <RE/A/Actor.h>
#include <RE/B/BSContainer.h>
#include <RE/I/InventoryChanges.h>
#include <RE/I/InventoryEntryData.h>
#include <RE/T/TESObjectARMO.h>
#include <functional>
#include <vector>

inline RE::InventoryChanges::IItemChangeVisitor::~IItemChangeVisitor() {}

namespace LIBC_NAMESPACE_DECL
{
    struct ArmorItemVisitor : RE::InventoryChanges::IItemChangeVisitor
    {
        std::vector<RE::TESObjectARMO *> armors{};

        ~ArmorItemVisitor() override {}

        auto Visit(RE::InventoryEntryData *a_entryData) -> RE::BSContainer::ForEachResult override;
    };

    class ArmorGenerator
    {
    public:
        virtual void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) = 0;

    protected:
        static auto IsArmorCanDisplay(RE::TESObjectARMO *armor) -> bool;
    };

    class InventoryArmorGenerator : public ArmorGenerator
    {
        RE::Actor *actor = nullptr;

    public:
        InventoryArmorGenerator(RE::Actor *actor) : actor(actor) {}

        void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;
    };

    class CarriedArmorGenerator : public ArmorGenerator
    {
        RE::Actor *actor = nullptr;

    public:
        CarriedArmorGenerator(RE::Actor *actor) : actor(actor) {}

        void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;
    };

    class BasicArmorGenerator : public ArmorGenerator
    {
    public:
        void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;
    };

}