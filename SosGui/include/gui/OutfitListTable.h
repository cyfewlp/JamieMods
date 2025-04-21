#pragma once

#include "GuiContext.h"
#include "common/config.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/BaseGui.h"
#include "gui/OutfitEditPanel.h"
#include "gui/Popup.h"
#include "imgui.h"
#include "service/OutfitService.h"
#include "util/PageUtil.h"

#include <RE/A/Actor.h>
#include <coroutine.h>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    class OutfitListTable : public BaseGui
    {
        static constexpr int                   OUTFIT_NAME_MAX_BYTES = 256;
        static constexpr SosUiData::OutfitPair DEFAULT_INVALID_PAIR  = {INVALID_ID, nullptr};

        SosUiData     &m_uiData;
        OutfitService &m_outfitService;

        Popup::DeleteOutfitPopup m_DeleteOutfitPopup{};
        SosUiData::OutfitPair    m_wantEdit = DEFAULT_INVALID_PAIR;
        SosUiData::OutfitPair    m_click    = DEFAULT_INVALID_PAIR;
        OutfitEditPanel          m_editPanel;
        util::PageUtil           m_outfitLisPage;

    public:
        OutfitListTable(SosUiData &uiData, OutfitService &outfitService)
            : m_uiData(uiData), m_outfitService(outfitService), m_editPanel(m_outfitService)
        {
        }

        void Render(GuiContext &guiContext, ImVec2 childSize);

        void FocusOutfit(const OutfitId &id);

        void Refresh() override;
        void Close() override;

    private:
        CoroutinePromise operator<<(CoroutineTask &&task)
        {
            co_await task;
        }

        void RenderChildContent(GuiContext &guiContext);

        /**
         * open a context menu if user right-click current row
         * @param acceptEdit modify to true if accept "Edit this outfit"
         * @return true if the context menu is open.
         */
        bool OpenContextMenu(GuiContext &guiContext, const SosUiOutfit &outfit, bool &acceptEdit);

        bool EditingPanel(const SosUiData::OutfitPair &wantEdit)
        {
            return m_editPanel.Render(wantEdit);
        }

        bool DeletePopup(const SosUiData::OutfitPair &clicked);

        void OnAcceptEditOutfit(const SosUiData::OutfitPair &wantEdit);
        void OnAcceptOutfitForState(GuiContext &guiContext, const std::string &outfitName);
        void OnAcceptActiveOutfit(RE::Actor *editingActor, OutfitId id, const std::string &outfitName);

        bool IsValidOutfit(SosUiData::OutfitPair &pair) const
        {
            return m_uiData.GetOutfitList().HasOutfit(pair.first);
        }
    };
}