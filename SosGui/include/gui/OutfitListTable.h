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

namespace
LIBC_NAMESPACE_DECL
{
class OutfitListTable final : public BaseGui
{
    static constexpr int MAX_OUTFIT_NAME_BYTES = 256;
    static constexpr SosUiData::OutfitPair DEFAULT_INVALID_PAIR = {INVALID_OUTFIT_ID, nullptr};
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

    SosUiData &m_uiData;
    OutfitService &m_outfitService;

    Popup::DeleteOutfitPopup m_DeleteOutfitPopup{};
    SosUiData::OutfitPair m_wantEdit = DEFAULT_INVALID_PAIR;
    MultiSelection m_outfitMultiSelection;
    OutfitEditPanel m_editPanel;

    bool m_onlyShowFavorites = false;
    std::array<char, MAX_OUTFIT_NAME_BYTES> m_outfitNameBuf{};
    OutfitDebounceInput m_outfitFilterInput{};

public:
    OutfitListTable(SosUiData &uiData, OutfitService &outfitService)
        : m_uiData(uiData), m_outfitService(outfitService), m_editPanel(m_uiData, m_outfitService) {}

    void Render(RE::Actor *editingActor);
    void Refresh() override;
    void Close() override;
    void OnSelectActor(const RE::Actor *actor);

private:
    void OnRefreshOutfitList();
    void DrawSidebar(RE::Actor *editingActor);

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

    void OnAcceptEditOutfit(const SosUiOutfit *lastEdit, const SosUiData::OutfitPair &wantEdit);
    void OnAcceptActiveOutfit(RE::Actor *editingActor, OutfitId id, const std::string &outfitName) const;
    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite);
    void OnAcceptDeleteOutfits();

    bool IsValidOutfit(const SosUiData::OutfitPair &pair) const
    {
        return m_uiData.GetOutfitList().HasOutfit(pair.first);
    }
};
}