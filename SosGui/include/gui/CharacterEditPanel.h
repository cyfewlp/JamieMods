//
// Created by jamie on 2025/5/25.
//

#ifndef CHARACTEREDITPANEL_H
#define CHARACTEREDITPANEL_H

#include "autoswitch/ActorPolicyView.h"
#include "common/config.h"
#include "gui/BaseGui.h"

namespace LIBC_NAMESPACE_DECL
{
class CharacterEditPanel final : public BaseGui
{
public:
    void Focus() override;

    void OnRefresh() override { m_selectedActorIndex = 0; }

    void Cleanup() override { m_selectedActorIndex = 0; }

    void DrawOutfitSelectPopup(RE::Actor *const &selectedActor, SosUiData &uiData, const OutfitService &outfitService);
    void Draw(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);

    auto GetSelectedActorIndex() const -> int { return m_selectedActorIndex; }

private:
    std::unique_ptr<OutfitSelectPopup> m_outfitSelectPopup = nullptr;
    AutoSwitch::ActorPolicyView        m_autoSwitchOutfitView{};
    int                                m_selectedActorIndex = 0;

    void DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator);
    void DrawCharactersTable(SosUiData &uiData, const SosDataCoordinator &dataCoordinator);

    auto GetSelectedActor(SosUiData &uiData) const -> RE::Actor *;
};
} // namespace LIBC_NAMESPACE_DECL

#endif // CHARACTEREDITPANEL_H
