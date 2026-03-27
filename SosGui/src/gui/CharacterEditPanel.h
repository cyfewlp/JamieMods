//
// Created by jamie on 2025/5/25.
//

#include "autoswitch/ActorPolicyView.h"
#include "gui/BaseGui.h"

namespace SosGui
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

    void DrawCharactersPanel(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);
    void DrawCharactersTable(SosUiData &uiData, const SosDataCoordinator &dataCoordinator, const OutfitService &outfitService);

    auto GetSelectedActor(SosUiData &uiData) const -> RE::Actor *;
};

} // namespace SosGui
