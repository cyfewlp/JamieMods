#pragma once

#include "GuiContext.h"
#include "SosUiData.h"
#include "common/config.h"
#include "data/SosUiOutfit.h"
#include "gui/BaseGui.h"
#include "gui/OutfitEditPanel.h"
#include "gui/Popup.h"
#include "gui/SosDataCoordinator.h"
#include "gui/Table.h"

#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
    class OutfitListTable : public BaseGui
    {
        static constexpr int                   OUTFIT_NAME_MAX_BYTES = 256;
        static constexpr SosUiData::OutfitPair DEFAULT_INVALID_PAIR  = {SosUiOutfit::INVALID_ID, nullptr};

        SosUiData          &m_uiData;
        SosDataCoordinator &m_dataCoordinator;

        Popup::DeleteOutfitPopup m_DeleteOutfitPopup;
        SosUiData::OutfitPair    m_wantEdit = DEFAULT_INVALID_PAIR;
        SosUiData::OutfitPair    m_click    = DEFAULT_INVALID_PAIR;
        OutfitEditPanel          m_editPanel;
        TableContext<1>          m_outfitListTable;

    public:
        OutfitListTable(SosUiData &uiData, SosDataCoordinator &dataCoordinator)
            : m_uiData(uiData), m_dataCoordinator(dataCoordinator), m_editPanel(m_uiData, m_dataCoordinator),
              m_outfitListTable(TableContext<1>::Create("##OutfitLists", {"$SkyOutSys_MCM_OutfitList"}))
        {
            m_outfitListTable.Sortable().NoHostExtendX();
        }

        void Render(GuiContext &guiContext, ImVec2 childSize);

        virtual void Refresh() override;
        virtual void Close() override;

    private:
        CoroutinePromise operator<<(CoroutineTask &&task) { co_await task; }

        void RenderChildContent(GuiContext &guiContext);

        /**
         * open a context menu if user right-click current row
         * @param outfit that current row outfit in loop
         * @param acceptEdit modify to true if accept "Edit this outfit"
         * @return true if the context menu is open.
         */
        bool OpenContextMenu(GuiContext &guiContext, const std::string &outfitName, bool &acceptEdit);

        inline bool EditingPanel(const SosUiData::OutfitPair &wantEdit) { return m_editPanel.Render(wantEdit); }

        bool DeletePopup(const SosUiData::OutfitPair &clicked);

        void OnAcceptEditOutfit(const SosUiData::OutfitPair &wantEdit);
        void OnAcceptOutfitForState(GuiContext &guiContext, const std::string &outfitName);
        void OnAcceptActiveOutfit(RE::Actor *editingActor, const std::string &outfitName);

        inline bool IsValidOutfit(SosUiData::OutfitPair &pair) { return m_uiData.HasOutfit(pair.first); }
    };
}