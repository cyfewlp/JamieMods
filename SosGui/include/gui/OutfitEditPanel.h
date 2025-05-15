#pragma once

#include "BaseGui.h"
#include "common/config.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/ArmorView.h"
#include "gui/context.h"
#include "popup/Popup.h"
#include "service/OutfitService.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/T/TESObjectARMO.h>
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
        RE::Actor                      *selectedActor = nullptr;
    } m_armorGeneratorTabBar;

    struct EditContext
    {
        bool ShowAllSlotOutfitArmors = false;
        bool dirty                   = true;

        void Clear();
    } m_editContext = {};

    struct OutfitModifyRequest : Popup::ModalPopup
    {
        OutfitId id;

        explicit OutfitModifyRequest(const char *const nameKey, const OutfitId id) : ModalPopup(nameKey), id(id) {}

        virtual void OnConfirm([[maybe_unused]] OutfitEditPanel *editPanel) {};
    };

    class DeleteRequest final : public OutfitModifyRequest
    {
    public:
        const Armor *armor;

        DeleteRequest(const OutfitId id, const Armor *const armor)
            : OutfitModifyRequest("$SosGui_Popup_DeleteArmor", id), armor(armor)
        {
        }

        void OnConfirm(OutfitEditPanel *editPanel) override;

    protected:
        void DoDraw(SosUiData &uiData, bool &confirmed) override;
    };

    class OverrideArmorRequest final : public OutfitModifyRequest
    {
    public:
        const Armor *armor;

        explicit OverrideArmorRequest(const OutfitId id, const Armor *const armor)
            : OutfitModifyRequest("$SosGui_Popup_OverrideArmor", id), armor(armor)
        {
        }

        void OnConfirm(OutfitEditPanel *editPanel) override;

    protected:
        void DoDraw(SosUiData &uiData, bool &confirmed) override;
    };

    class BatchAddArmorsRequest final : public OutfitModifyRequest
    {
    public:
        explicit BatchAddArmorsRequest(const OutfitId id) : OutfitModifyRequest("$SosGui_Popup_BatchAddArmors", id) {}

        void OnConfirm(OutfitEditPanel *editPanel) override;

    protected:
        void DoDraw(SosUiData &uiData, bool &confirmed) override;
    };

    class SlotPolicyHelp final : public Popup::ModalPopup
    {
    public:
        explicit SlotPolicyHelp() : ModalPopup("$SkyOutSys_OEdit_SlotPolicyHelp") {}

        bool Draw(SosUiData &uiData, bool &confirmed, ImGuiWindowFlags flags = 0) override;

    protected:
        void DoDraw(SosUiData &uiData, bool &confirmed) override;
    };

    ArmorView      m_armorView{};
    std::string    m_windowTitle;
    SosUiData     &m_uiData;
    OutfitService &m_outfitService;

public:
    explicit OutfitEditPanel(SosUiData &uiData, OutfitService &outfitService)
        : m_uiData(uiData), m_outfitService(outfitService)
    {
    }

    void Show() override
    {
        BaseGui::Show();
        m_armorView.init();
    }

    void OnRefresh() override;
    void Cleanup() override;

    void Draw(Context &context, const EditingOutfit &editingOutfit);
    void DrawOutfitPanel(Context &context, const EditingOutfit &editingOutfit);
    void OnSelectActor(const RE::Actor *actor, const EditingOutfit &editingOutfit);
    void OnSelectOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editing);

    // Only one modal popup can be rendered in the same time.
    bool OnModalPopupConfirmed(Popup::ModalPopup *modalPopup);

private:
    enum class Error
    {
        delete_armor_from_unknown_outfit_id,
        add_armor_to_unknown_outfit_id,
    };

    static void PushError(SosUiData &uiData, Error error);

    void DoDraw(Context &context, const EditingOutfit &editingOutfit);
    void DrawSideBar(const SosUiOutfit *editingOutfit);
    void UpdateWindowTitle(const std::string &outfitName);

    void DrawOutfitArmors(Context &context, const EditingOutfit &editingOutfit);
    void HighlightConflictSlot(Slot slot) const;
    void SlotPolicyCombo(const EditingOutfit &editingOutfit, const uint32_t &slotIdx) const;

    auto GetGenerator() const -> ArmorGenerator *
    {
        return m_armorGeneratorTabBar.generator.get();
    }

    using DrawArmorEntry = std::function<void(const Armor *armor, size_t index)>;
    void DrawArmorGeneratorTabBar(const SosUiOutfit *editingOutfit);
    void DrawArmorViewFilter(const SosUiOutfit *editingOutfit);
    void DrawArmorView(Context &context, const EditingOutfit &editingOutfit);
    void DrawArmorViewContent(
        Context &context, const EditingOutfit &editingOutfit, const std::vector<ArmorView::RankedArmor> &viewData
    );
    void DrawArmorViewTableContent(
        const std::vector<ArmorView::RankedArmor> &viewData, const DrawArmorEntry &drawArmorEntry
    );
    void DrawArmorViewModNameFilterer(const SosUiOutfit *editingOutfit);
    void DrawArmorViewSlotFilterer(const SosUiOutfit *editing);

    void OnAcceptAddArmorToOutfit(Context &context, const EditingOutfit &editingOutfit, const Armor *armor);

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