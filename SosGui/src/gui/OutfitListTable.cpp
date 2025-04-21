#include "gui/OutfitListTable.h"

#include "GuiContext.h"
#include "Translation.h"
#include "common/config.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/Table.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"
#include "util/PageUtil.h"

#include <RE/A/Actor.h>
#include <SosDataType.h>
#include <array>
#include <string>
#include <utility>

namespace LIBC_NAMESPACE_DECL
{
    void OutfitListTable::Refresh()
    {
        m_click    = DEFAULT_INVALID_PAIR;
        m_wantEdit = DEFAULT_INVALID_PAIR;
    }

    void OutfitListTable::Close()
    {
        m_click    = DEFAULT_INVALID_PAIR;
        m_wantEdit = DEFAULT_INVALID_PAIR;
    }

    void OutfitListTable::Render(GuiContext &guiContext, ImVec2 childSize)
    {
        constexpr auto flags = ImGuiChildFlags_None;
        if (ImGuiUtil::BeginChild("$SkyOutSys_MCMHeader_OutfitList", childSize, flags))
        {
            RenderChildContent(guiContext);
        }
        // must out of BeginChild braces because when dock window, BeginChild will be return false;
        if (IsValidOutfit(m_click) && DeletePopup(m_click))
        {
            m_click = DEFAULT_INVALID_PAIR;
        }
        if (IsValidOutfit(m_wantEdit) && EditingPanel(m_wantEdit))
        {
            m_wantEdit = DEFAULT_INVALID_PAIR;
        }
        ImGui::EndChild();
    }

    void OutfitListTable::FocusOutfit(const OutfitId &id)
    {
        auto &outfitList = m_uiData.GetOutfitList();
        auto  pos        = outfitList.Rank(id);
        if (pos >= outfitList.size())
        {
            return;
        }
        m_outfitLisPage.TurnTo(pos);
    }

    void OutfitListTable::RenderChildContent(GuiContext &guiContext)
    {
        //////////////////////////////////////////////////////////
        // Create Outfit widgets
        static std::array<char, OUTFIT_NAME_MAX_BYTES> outfitNameBuf;
        ImGui::InputText("##CreateNewInput", outfitNameBuf.data(), outfitNameBuf.size());

        ImGui::BeginDisabled(outfitNameBuf[0] == '\0');
        if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
        {
            *this << m_outfitService.CreateOutfit(outfitNameBuf.data());
        }

        ImGui::SameLine();
        if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
        {
            *this << m_outfitService.CreateOutfitFromWorn(outfitNameBuf.data());
        }
        ImGui::EndDisabled();

        if (ImGuiUtil::Button("$SosGui_Refresh{$SkyOutSys_MCM_OutfitList}"))
        {
            m_outfitService.GetOutfitList();
        }

        //////////////////////////////////////////////////////////
        // Table Content
        auto &outfits = m_uiData.GetOutfitList();
        ImGui::SameLine();
        static bool onlyShowFavorites = false;
        if (ImGuiUtil::CheckBox("$SosGui_CheckBox_OnlyShowFavorites", &onlyShowFavorites))
        {
            outfits.OnlyFavoriteOutfits(onlyShowFavorites);
        }
        static int selectedIdx = -1;

        util::RenderPageWidgets(m_outfitLisPage);
        if (!TableBuilder("##OutfitLists").Sortable().NoHostExtendX().Begin(3))
        {
            return;
        }

        // clang-format off
        TableHeadersBuilder()
            .Column("##Number").NoSort().WidthFixed().NoHide()
            .Column("$SkyOutSys_MCM_OutfitList").DefaultSort().WidthStretch()
            .Column("##ActiveMark").NoSort().WidthFixed()
            .CommitHeadersRow();
        // clang-format on

        if (auto *sortSpecs = ImGui::TableGetSortSpecs(); sortSpecs != nullptr)
        {
            if (sortSpecs->SpecsDirty)
            {
                const auto direction = sortSpecs->Specs[0].SortDirection;
                m_outfitLisPage.SetAscendSort(direction == ImGuiSortDirection_Ascending);
                sortSpecs->SpecsDirty = false;
            }
        }

        m_outfitLisPage.SetItemCount(outfits.size());
        auto rowAction = [&](const auto &outfit, size_t index) {
            ImGuiUtil::PushIdGuard idGuard(index);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%zu", index + 1);

            const auto &outfitName = outfit.GetName();
            ImGui::TableNextColumn(); // outfit name column
            {
                constexpr auto flags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;

                bool const isSelected = static_cast<size_t>(selectedIdx) == index;
                if (ImGui::Selectable(outfitName.data(), isSelected, flags))
                {
                    selectedIdx = isSelected ? -1 : index;
                }

                bool acceptEdit = false;
                if (OpenContextMenu(guiContext, outfit, acceptEdit))
                {
                    m_click = std::make_pair(outfit.GetId(), &outfit);
                }
                if (acceptEdit)
                {
                    auto pair = std::make_pair(outfit.GetId(), &outfit);
                    OnAcceptEditOutfit(pair);
                    m_wantEdit = pair;
                }
            }

            ImGui::TableNextColumn(); // active outfit hint column
            {
                if (m_uiData.GetActorOutfitMap().IsActorOutfit(guiContext.editingActor, outfit.GetId()))
                {
                    ImGuiUtil::AddItemRectWithCol(ImGuiCol_HeaderActive, 2.5F);
                    ImGuiUtil::Text("$SkyOutSys_OutfitBrowser_ActiveMark");
                }
                else
                {
                    ImGui::Text("");
                }
                if (outfit.IsFavorite())
                {
                    ImGui::SameLine();
                    ImGui::Text("\xe2\xad\x90");
                }
            }
        };

        auto pageRange = m_outfitLisPage.PageRange();
        outfits.for_each(m_outfitLisPage.IsAscend(), pageRange.first, pageRange.second, rowAction);
        ImGui::EndTable();
    }

    bool OutfitListTable::OpenContextMenu(GuiContext &guiContext, const SosUiOutfit &outfit, bool &acceptEdit)
    {
        if (!ImGui::BeginPopupContextItem("##OutfitListContextMenu"))
        {
            return false;
        }
        const auto &outfitName = outfit.GetName();

        if (ImGui::MenuItem(Translation::Translate("$SosGui_OpenOutfitEditPanel", outfitName).c_str()))
        {
            acceptEdit = true;
        }
        ImGui::Separator();
        bool noEditingActor = guiContext.editingActor == nullptr;
        ImGui::BeginDisabled(noEditingActor);
        {
            if (noEditingActor)
            {
                ImGuiUtil::Text("$SosGui_SelectHint{$Characters}");
            }
            const auto *actorName = noEditingActor ? "" : guiContext.editingActor->GetName();
            ImGui::Text("%s", Translation::Translate("$SosGui_EditingActor", actorName).c_str());
            if (m_uiData.GetActorOutfitMap().IsActorOutfit(guiContext.editingActor, outfit.GetId()))
            {
                if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOff"))
                {
                    OnAcceptActiveOutfit(guiContext.editingActor, INVALID_ID, "");
                }
            }
            if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOn"))
            {
                OnAcceptActiveOutfit(guiContext.editingActor, outfit.GetId(), outfitName);
            }
            ImGui::BeginDisabled(guiContext.editingState == StateType::None);
            {
                if (ImGui::MenuItem("AutoSwitch: Use this outfit"))
                {
                    OnAcceptOutfitForState(guiContext, outfitName);
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndDisabled();
        ImGui::Separator();
        if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_Delete"))
        {
            m_DeleteOutfitPopup.Open();
        }
        if (ImGui::MenuItem("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
        return true;
    }

    bool OutfitListTable::DeletePopup(const SosUiData::OutfitPair &clicked)
    {
        bool isConfirmDelete = false;
        bool justClosed      = m_DeleteOutfitPopup.Render(clicked.second->GetName(), isConfirmDelete);
        if (isConfirmDelete)
        {
            *this << m_outfitService.DeleteOutfit(clicked.first, clicked.second->GetName());
        }
        return justClosed;
    }

    void OutfitListTable::OnAcceptEditOutfit(const SosUiData::OutfitPair &wantEdit)
    {
        *this << m_outfitService.GetOutfitArmors(wantEdit.first, wantEdit.second->GetName());
        *this << m_outfitService.GetSlotPolicy(wantEdit.first, wantEdit.second->GetName());
        m_editPanel.ShowWindow(wantEdit.second->GetName());
    }

    void OutfitListTable::OnAcceptOutfitForState(GuiContext &guiContext, const std::string &outfitName)
    {
        *this << m_outfitService.SetActorStateOutfit(guiContext.editingActor, guiContext.editingState, outfitName);
    }

    void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, OutfitId id, const std::string &outfitName)
    {
        *this << m_outfitService.SetActorOutfit(editingActor, id, outfitName);
    }
}
