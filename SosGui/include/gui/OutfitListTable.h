#pragma once

#include "common/config.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/OutfitEditPanel.h"
#include "popup/Popup.h"
#include "service/OutfitService.h"
#include "util/ImGuiUtil.h"

#include <string>

namespace LIBC_NAMESPACE_DECL
{
class OutfitListTable final : public BaseGui
{
    static constexpr int        MAX_OUTFIT_NAME_BYTES = 256;
    static inline SosUiOutfit   OUTFIT                = SosUiOutfit(UNTITLED_OUTFIT_ID, "Untitled");
    static inline EditingOutfit UNTITLED_OUTFIT{OUTFIT};
    using DrawOutfitEntry = std::function<void(const SosUiOutfit *, size_t)>;

    struct OutfitDebounceInput final : ImGuiUtil::DebounceInput
    {
        std::vector<const SosUiOutfit *> viewData;

        OutfitDebounceInput() = default;

        void OnInput() override
        {
            viewData.clear();
            DebounceInput::OnInput();
        }

        void OnUpdate(const OutfitList &outfitList, bool onlyFavorites);

        void Clear() override
        {
            DebounceInput::Clear();
            viewData.clear();
        }
    };

    class CreateOutfitPopup final : public Popup::ModalPopup
    {
    public:
        enum class Flags : std::uint8_t
        {
            NONE = 0,
            CREATE_EMPTY,
            CREATE_FROM_WORN
        };

        explicit CreateOutfitPopup() : ModalPopup("$SosGui_CreateOutfit") {}

        [[nodiscard]] auto GetFlags() const -> Flags
        {
            return m_flags;
        }

        auto GetOutfitName() const -> std::string
        {
            return std::string(m_outfitNameBuf.data(), m_outfitNameBuf.size());
        }

    protected:
        void DoDraw(SosUiData &uiData, bool &confirmed) override;

    private:
        std::array<char, MAX_OUTFIT_NAME_BYTES> m_outfitNameBuf{};
        Flags                                   m_flags = Flags::NONE;
    };

    SosUiData          &m_uiData;
    OutfitService      &m_outfitService;
    OutfitEditPanel    &m_editPanel;
    EditingOutfit       m_wantEdit = UNTITLED_OUTFIT;
    MultiSelection      m_outfitMultiSelection;
    OutfitDebounceInput m_outfitFilterInput{};

public:
    OutfitListTable(SosUiData &uiData, OutfitService &outfitService, OutfitEditPanel &editPanel)
        : m_uiData(uiData), m_outfitService(outfitService), m_editPanel(editPanel)
    {
    }

    void Show() override;
    void Focus() override;
    void OnRefresh() override;
    void Cleanup() override;

    void Draw(Context &context, RE::Actor *editingActor);
    void OnSelectActor(const RE::Actor *actor) const;
    // Only one modal popup can be rendered in the same time.
    bool OnModalPopupConfirmed(Popup::ModalPopup *modalPopup) const;

    auto GetEditingOutfit() -> EditingOutfit &
    {
        return m_wantEdit;
    }

private:
    // refresh, filterer, favorite checkbox...
    void DrawToolWidgets();
    void DrawOutfitTable(Context &context, RE::Actor *editingActor);
    void DrawOutfitTableContent(Context &context, RE::Actor *editingActor);
    void OnToggleFavorite(std::vector<const SosUiOutfit *> &viewData, size_t index) const;

    static void PreDrawOutfits(ImGuiListClipper &clipper, MultiSelection &selection);
    static void PostDrawOutfits(MultiSelection &selection);
    void DrawOutfitTableContent(std::vector<const SosUiOutfit *> outfitView, bool ascend, const DrawOutfitEntry &drawOutfitEntry);

    /**
     * open a context menu if user right-clicks current row
     * @return true if the context menu is open.
     */
    void OpenContextMenu(
        Context &context, uint32_t selectedItemCount, RE::Actor *editingActor, const SosUiOutfit *outfit,
        __out bool &acceptRename
    );

    void OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const;
    void OnAcceptActiveOutfit(RE::Actor *editingActor, OutfitId id, const std::string &outfitName) const;
    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite);
    void OnAcceptDeleteOutfits();
};
}