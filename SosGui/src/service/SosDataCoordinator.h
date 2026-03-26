#pragma once

#include "data/SosUiData.h"
#include "service/OutfitService.h"
#include "task.h"

#include <functional>

namespace SosGui
{
class SosDataCoordinator
{
    SosUiData     &m_uiData;
    OutfitService &m_outfitService;
    using OnComplete = std::function<void()>;
    using Variable   = RE::BSScript::Variable;
    using Armor      = RE::TESObjectARMO;

public:
    explicit SosDataCoordinator(SosUiData &uiData, OutfitService &outfitService) : m_uiData(uiData), m_outfitService(outfitService) {}

    auto RequestEnable(bool isEnabled) const -> Task;
    auto QueryIsEnable() const -> Task;
    auto RequestImportSettings() const -> Task;
    auto RequestExportSettings() const -> Task;
    auto RequestUpdateActorAutoSwitchState(RE::Actor *actor) const -> Task;
    auto RequestSetActorAutoSwitchState(const RE::Actor *actor, bool enabled) const -> Task;
    auto RequestActorList() const -> Task;
    auto RequestAddActor(RE::Actor *actor) const -> Task;
    auto RequestRemoveActor(RE::Actor *actor) const -> Task;
    auto RequestNearActorList() const -> Task;
    auto Refresh() const -> Task;

    static auto HasQuickSlotSpell() -> bool;
};
} // namespace SosGui
