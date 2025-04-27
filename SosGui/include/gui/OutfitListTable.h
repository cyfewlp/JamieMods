#pragma once

#include "GuiContext.h"
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
    static constexpr int OUTFIT_NAME_MAX_BYTES = 256;
    static constexpr SosUiData::OutfitPair DEFAULT_INVALID_PAIR = {INVALID_OUTFIT_ID, nullptr};

    struct outfit_debounce_input final : ImGuiUtil::debounce_input
    {
        std::vector<const SosUiOutfit *> outfitView;

        outfit_debounce_input(const char *label, const char *hintText) : debounce_input(label, hintText) {}

        void onInput() override
        {
            outfitView.clear();
            debounce_input::onInput();
        }

        void onUpdate(const OutfitList &outfitList, bool onlyFavorites);

        void clear() override
        {
            debounce_input::clear();
            outfitView.clear();
        }
    };

    SosUiData &m_uiData;
    OutfitService &m_outfitService;

    Popup::DeleteOutfitPopup m_DeleteOutfitPopup{};
    SosUiData::OutfitPair m_wantEdit = DEFAULT_INVALID_PAIR;
    SosUiData::OutfitPair m_click = DEFAULT_INVALID_PAIR;
    MultiSelection m_outfitMultiSelection;
    OutfitEditPanel m_editPanel;
    bool m_onlyShowFavorites = false;
    outfit_debounce_input m_outfitFilterInput{"##filter", "filter outfit"};

public:
    OutfitListTable(SosUiData &uiData, OutfitService &outfitService)
        : m_uiData(uiData), m_outfitService(outfitService), m_editPanel(m_outfitService) {}

    void Render(GuiContext &guiContext);
    void Refresh() override;
    void Close() override;
    void OnSelectActor(const RE::Actor*actor);

private:
    using OutfitDrawAction = std::function<void(const SosUiOutfit &, size_t)>;

    void RefreshOutfitList();
    void Sidebar(GuiContext &guiContext);
    static void PreDrawOutfits(ImGuiListClipper &clipper, MultiSelection &selection);
    static void PostDrawOutfits(MultiSelection &selection);
    void DrawOutfits(std::vector<const SosUiOutfit *> outfitView, bool ascend, const OutfitDrawAction &drawAction);

    /**
     * open a context menu if user right-click current row
     * @return true if the context menu is open.
     */
    bool OpenContextMenu(const GuiContext &guiContext, const SosUiOutfit &outfit);

    bool EditingPanel(const SosUiData::OutfitPair &wantEdit)
    {
        return m_editPanel.Render(wantEdit);
    }

    bool DeletePopup(const SosUiData::OutfitPair &clicked);

    void OnAcceptEditOutfit(const SosUiOutfit *lastEdit, const SosUiData::OutfitPair &wantEdit);
    void OnAcceptActiveOutfit(RE::Actor *editingActor, OutfitId id, const std::string &outfitName);
    // check MultiSelection and set all selected outfit to favorite
    void OnAcceptSetFavoriteOutfits(bool toFavorite);

    bool IsValidOutfit(const SosUiData::OutfitPair &pair) const
    {
        return m_uiData.GetOutfitList().HasOutfit(pair.first);
    }
};
}