#pragma once

#include "common/config.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/OutfitEditPanel.h"
#include "gui/Popup.h"
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
    using OutfitDrawAction = std::function<void(const SosUiOutfit &, size_t)>;

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

        void clear() override
        {
            DebounceInput::clear();
            viewData.clear();
        }
    };

    SosUiData                              &m_uiData;
    OutfitService                          &m_outfitService;
    OutfitEditPanel                        &m_editPanel;
    EditingOutfit                           m_wantEdit = UNTITLED_OUTFIT;
    MultiSelection                          m_outfitMultiSelection;
    std::array<char, MAX_OUTFIT_NAME_BYTES> m_outfitNameBuf{};
    OutfitDebounceInput                     m_outfitFilterInput{};

    std::unique_ptr<Popup::DeleteOutfitPopup> m_deleteOutfitPopup = nullptr;

public:
    OutfitListTable(SosUiData &uiData, OutfitService &outfitService, OutfitEditPanel &editPanel)
        : m_uiData(uiData), m_outfitService(outfitService), m_editPanel(editPanel)
    {
    }

    void Cleanup() override;

    void Draw(RE::Actor *editingActor);
    void OnSelectActor(const RE::Actor *actor) const;

    auto GetEditingOutfit() -> EditingOutfit &
    {
        return m_wantEdit;
    }

private:
    void DoDraw(RE::Actor *editingActor);

    static void PreDrawOutfits(ImGuiListClipper &clipper, MultiSelection &selection);
    static void PostDrawOutfits(MultiSelection &selection);
    void DrawOutfits(std::vector<const SosUiOutfit *> outfitView, bool ascend, const OutfitDrawAction &drawAction);

    /**
     * open a context menu if user right-clicks current row
     * @return true if the context menu is open.
     */
    void OpenContextMenu(uint32_t selectedItemCount, RE::Actor *editingActor, const SosUiOutfit &outfit,
                         __out bool &acceptRename);
    void DrawDeletePopup();

    void OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const;
    void OnAcceptActiveOutfit(RE::Actor *editingActor, OutfitId id, const std::string &outfitName) const;
    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite);
    void OnAcceptDeleteOutfits();
};
}