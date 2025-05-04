#pragma once

#include "BaseGui.h"
#include "common/config.h"
#include "data/ArmorContainer.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "gui/Popup.h"
#include "service/OutfitService.h"
#include "util/ImGuiUtil.h"
#include "util/PageUtil.h"
#include "widgets.h"

#include "data/SosUiOutfit.h"
#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <expected>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
class OutfitEditPanel final : public BaseGui
{
public:
    using Slot            = RE::BIPED_MODEL::BipedObjectSlot;
    using Armor           = RE::TESObjectARMO;
    using SlotEnumeration = SKSE::stl::enumeration<Slot, uint32_t>;

    static constexpr int MAX_FILTER_ARMOR_NAME = 256;
    static constexpr int SLOT_COUNT            = 32;
    static constexpr int SOS_SLOT_OFFSET       = 30;

private:
    static auto get_slot_name_key(uint32_t slotPos) -> std::string;

    static bool IsArmorNonPlayable(const Armor *armor)
    {
        return (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0;
    }

    struct ArmorFilter final : ImGuiUtil::DebounceInput
    {
        bool mustPlayable = false;

        bool PassFilter(const Armor *armor) const;

        bool Draw();
    };

    enum class error : uint8_t
    {
        unassociated_armor,
        armor_already_exists,
        armor_not_exist,
    };

    static auto ToErrorMessage(const error error) -> std::string
    {
        switch (error)
        {
            case error::unassociated_armor:
                return "Unassociated Armor: Missing in container, try reopen menu to reslove.";
            case error::armor_already_exists:
                return "Armor already exists in view";
            case error::armor_not_exist:
                return "Armor not exist in view";
            default:
                return "Unknown error";
        }
    }

    struct ArmorView
    {
        std::bitset<RE::BIPED_OBJECT::kEditorTotal>    selectedFilterSlot{};
        ArmorContainer                                 armorContainer{};
        std::vector<Armor *>                           viewData{};
        std::unordered_map<std::string_view, uint32_t> modRefCounter; // only update when generator update
        ArmorFilter                                    armorFilter{};
        std::string_view                               modNameFilterer = "";
        bool                                           checkAllSlot    = true; // default shows all armor slot
        std::array<uint16_t, SLOT_COUNT>               slotCounter{};
        MultiSelection                                 multiSelection{};
        SlotEnumeration                                multiSelectedSlot{}; // be used to highlight conflict armors
        uint32_t                                       availableArmorCount = 0;
        ArmorGenerator                                *armorGenerator;

        ////////////////////////////////////////////////////////////////////
        // mod filterer -> slot-filterer -> armor-name filter
        void               init();
        void               clear();
        void               clearViewData();
        void               remove_armors_has_slot(Slot selectedSlots, Slot toRemoveSlot);
        void               add_armors_in_outfit(SosUiData &uiData, const SosUiOutfit *editingOutfit);
        void               remove_armors_in_outfit(const SosUiOutfit *editingOutfit);
        bool               filter(const Armor *armor) const;
        [[nodiscard]] auto add_armor(Armor *armor) -> std::expected<void, error>;
        bool               remove_armor(const Armor *armor);
        void               reset_counter();
        void               reset_view(ArmorGenerator *generator);
        auto               find_armor(const Armor *armor) const -> std::expected<size_t, error>;
        bool               no_select_any_slot() const;
    } m_armorView = {};

    struct ArmorGeneratorTabBar
    {
        std::unique_ptr<ArmorGenerator> generator     = std::make_unique<BasicArmorGenerator>();
        RE::Actor                      *selectedActor = nullptr;
    } m_armorGeneratorTabBar;

    struct EditContext
    {
        bool                            armorListShowAllSlotArmors  = false;
        RE::Actor                      *armorGeneratorSelectedActor = nullptr;
        std::unique_ptr<ArmorGenerator> activeArmorGenerator        = std::make_unique<BasicArmorGenerator>();
        // be used on click add(candidate table)/delete(armor table)
        bool dirty = true;
        void Clear();
    } m_editContext = {};

    std::string               m_windowTitle;
    SosUiData                &m_uiData;
    OutfitService            &m_outfitService;
    Popup::DeleteArmorPopup   m_DeleteArmorPopup{};
    Popup::ConflictArmorPopup m_ConflictArmorPopup{};
    Popup::SlotPolicyHelp     m_slotPolicyHelp{};
    Popup::BatchAddArmors     m_batchAddArmorsPopUp{};

public:
    explicit OutfitEditPanel(SosUiData &uiData, OutfitService &outfitService)
        : m_uiData(uiData), m_outfitService(outfitService)
    {
    }

    void Render(const SosUiData::OutfitPair &wantEdit);
    void Refresh() override;
    void Close() override;
    void OnSelectActor(const RE::Actor *actor, const SosUiOutfit *editingOutfit);
    void OnSelectOutfit(const SosUiOutfit *lastEditOutfit, const SosUiOutfit *editingOutfit);

private:
    void DrawSideBar(const SosUiOutfit *outfit);
    void UpdateWindowTitle(const std::string &outfitName);

    void DrawOutfitArmors(const SosUiData::OutfitPair &wantEdit);
    void HighlightConflictSlot(Slot slot) const;
    void SlotPolicyCombo(const SosUiData::OutfitPair &wantEdit, const uint32_t &slotIdx) const;

    auto GetGenerator() const -> ArmorGenerator *
    {
        return m_armorGeneratorTabBar.generator.get();
    }

    void DrawArmorGeneratorTabBar();
    void DrawArmorViewTableContent(const std::vector<Armor *>                            &viewData,
                                   const std::function<void(Armor *armor, size_t index)> &drawAction);
    void DrawArmorViewFilter(const SosUiOutfit *outfit);
    void DrawArmorView(const SosUiData::OutfitPair &wantEdit, const std::vector<Armor *> &viewData);
    void DrawArmorViewModNameFilterer();
    void DrawArmorViewSlotFilterer(const SosUiOutfit *outfit);

    void BatchAddArmors(const SosUiData::OutfitPair &wantEdit);

    void RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit, const bool &fFilterPlayable) const;

    static void GetArmorGeneratorFromPolicy(OutfitAddPolicy policy, ArmorGenerator **generator);

    void OnAcceptAddArmorToOutfit(const SosUiData::OutfitPair &wantEdit, Armor *armor);

    void RenderPopups(const SosUiData::OutfitPair &wantEdit);

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