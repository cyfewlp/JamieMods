#pragma once

#include "BaseGui.h"
#include "common/config.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/ArmorView.h"
#include "gui/Popup.h"
#include "service/OutfitService.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/T/TESObjectARMO.h>
#include <string>

namespace
LIBC_NAMESPACE_DECL
{
class OutfitEditPanel final : public BaseGui
{
public:
    using Slot            = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor           = RE::TESObjectARMO;
    using SlotEnumeration = SKSE::stl::enumeration<Slot, uint32_t>;

    static constexpr int MAX_FILTER_ARMOR_NAME = 256;
    static constexpr int SOS_SLOT_OFFSET       = 30;

private:
    static auto get_slot_name_key(uint32_t slotPos) -> std::string;

    static bool IsArmorNonPlayable(const Armor *armor)
    {
        return (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0;
    }

    struct ArmorGeneratorTabBar
    {
        std::unique_ptr<ArmorGenerator> generator     = std::make_unique<BasicArmorGenerator>();
        RE::Actor *                     selectedActor = nullptr;
    } m_armorGeneratorTabBar;

    struct EditContext
    {
        bool                            armorListShowAllSlotArmors  = false;
        RE::Actor *                     armorGeneratorSelectedActor = nullptr;
        std::unique_ptr<ArmorGenerator> activeArmorGenerator        = std::make_unique<BasicArmorGenerator>();
        // be used on click add(candidate table)/delete(armor table)
        bool dirty = true;
        void Clear();
    } m_editContext = {};

    ArmorView                 m_armorView{};
    std::string               m_windowTitle;
    SosUiData &               m_uiData;
    OutfitService &           m_outfitService;
    Popup::DeleteArmorPopup   m_DeleteArmorPopup{};
    Popup::ConflictArmorPopup m_ConflictArmorPopup{};
    Popup::SlotPolicyHelp     m_slotPolicyHelp{};
    Popup::BatchAddArmors     m_batchAddArmorsPopUp{};

public:
    explicit OutfitEditPanel(SosUiData &uiData, OutfitService &outfitService)
        : m_uiData(uiData), m_outfitService(outfitService) {}

    void Show() override
    {
        BaseGui::Show();
        m_armorView.init();
    }

    void Draw(const EditingOutfit &editingOutfit);
    void DrawOutfitTabBarView(const EditingOutfit &editingOutfit);
    void DrawArmorInfo();
    void Refresh() override;
    void Close() override;
    void OnSelectActor(const RE::Actor *actor, const EditingOutfit &editingOutfit);
    void OnSelectOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editing);

private:
    void DoDraw(const EditingOutfit &editingOutfit);
    void DrawSideBar(const SosUiOutfit *editingOutfit);
    void UpdateWindowTitle(const std::string &outfitName);

    void DrawOutfitArmors(const EditingOutfit& editingOutfit);
    void HighlightConflictSlot(Slot slot) const;
    void SlotPolicyCombo(const EditingOutfit& editingOutfit, const uint32_t &slotIdx) const;

    auto GetGenerator() const -> ArmorGenerator *
    {
        return m_armorGeneratorTabBar.generator.get();
    }

    void DrawArmorGeneratorTabBar(const SosUiOutfit *editingOutfit);
    void DrawArmorViewTableContent(const std::vector<const Armor *> &                           viewData,
                                   const std::function<void(const Armor *armor, size_t index)> &drawAction);
    void DrawArmorViewFilter(const SosUiOutfit *editingOutfit);
    void DrawArmorView(const EditingOutfit& editingOutfit, const std::vector<const Armor *> &viewData);
    void DrawArmorViewModNameFilterer(const SosUiOutfit *editingOutfit);
    void DrawArmorViewSlotFilterer(const SosUiOutfit *editing);

    void BatchAddArmors(const EditingOutfit &editingOutfit);
    void AddArmorToOutfit(const EditingOutfit &editingOutfit, const Armor *armor) const;

    void OnAcceptAddArmorToOutfit(const EditingOutfit& editingOutfit, const Armor *armor);

    void RenderPopups(const EditingOutfit &editingOutfit);

    static auto IsArmorCanDisplay(const Armor *armor) -> bool;

    static auto ToSlot(uint32_t slotPos) -> Slot
    {
        return slotPos >= RE::BIPED_OBJECT::kEditorTotal ? Slot::kNone : static_cast<Slot>(1 << slotPos);
    }

    static auto ToSlot(const RE::BIPED_OBJECT equipIndex) -> Slot
    {
        if (equipIndex >= RE::BIPED_OBJECT::kEditorTotal)
        {
            return Slot::kNone;
        }
        return static_cast<Slot>(1 << equipIndex);
    }
};

}