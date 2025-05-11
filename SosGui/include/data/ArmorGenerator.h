#pragma once

#include "common/config.h"

#include <RE/A/Actor.h>
#include <RE/B/BSContainer.h>
#include <RE/I/InventoryChanges.h>
#include <RE/T/TESObjectARMO.h>
#include <functional>
#include <vector>

inline RE::InventoryChanges::IItemChangeVisitor::~IItemChangeVisitor() {}

namespace
LIBC_NAMESPACE_DECL
{
struct ArmorItemVisitor final : RE::InventoryChanges::IItemChangeVisitor
{
    std::vector<RE::TESObjectARMO *> armors{};

    ~ArmorItemVisitor() override {}

    auto Visit(RE::InventoryEntryData *a_entryData) -> RE::BSContainer::ForEachResult override;
};

class ArmorGenerator
{
public:
    virtual      ~ArmorGenerator() = default;
    virtual void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) = 0;
};

class FormIdArmorGenerator final : public ArmorGenerator
{
    RE::FormID armorFormId = 0;

public:
    explicit FormIdArmorGenerator(RE::FormID a_armorFormId) : armorFormId(a_armorFormId) {}

    void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;
};

class NearObjectsInventoryArmorGenerator final : public ArmorGenerator
{
    std::vector<RE::TESObjectREFR *> nearObjects{};
    size_t                           wantVisitIndex = 0;

public:
    void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;

    void Update();

    [[nodiscard]] auto NearObjects() const -> const std::vector<RE::TESObjectREFR *> &
    {
        return nearObjects;
    }

    void SetWantVisitIndex(const size_t index)
    {
        wantVisitIndex = index;
    }

    [[nodiscard]] constexpr auto WantVisitIndex() const -> size_t
    {
        return wantVisitIndex;
    }
};

class InventoryArmorGenerator final : public ArmorGenerator
{
    RE::Actor *actor = nullptr;

public:
    explicit InventoryArmorGenerator(RE::Actor *actor) : actor(actor) {}

    void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;
};

class CarriedArmorGenerator final : public ArmorGenerator
{
    RE::Actor *actor = nullptr;

public:
    explicit CarriedArmorGenerator(RE::Actor *actor) : actor(actor) {}

    void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;
};

class BasicArmorGenerator final : public ArmorGenerator
{
public:
    void for_each(std::function<void(RE::TESObjectARMO *armor)> &&action) override;
};

}