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

#include <RE/A/Actor.h>
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

    void OutfitListTable::Render(GuiContext &guiContext)
    {
        std::string windowName;
        if (m_wantEdit.second != nullptr)
        {
            windowName = std::format("Editing Outfit: {}###OutfitEditor", m_wantEdit.second->GetName());
        }
        else
        {
            windowName = "Outfit Editor###OutfitEditor";
        }
        ImGui::SetNextWindowSize({500, 300}, ImGuiCond_FirstUseEver);
        if (ImGui::Begin(windowName.c_str()))
        {
            constexpr auto flags = ImGuiUtil::ChildFlag().Borders().ResizeX().flags;
            if (ImGuiUtil::BeginChild("$SkyOutSys_MCMHeader_OutfitList", ImVec2(300, 0), flags))
            {
                RenderChildContent(guiContext);
            }
            // must out of BeginChild braces because when dock window, BeginChild will be return false;
            if (IsValidOutfit(m_click) && DeletePopup(m_click))
            {
                m_click = DEFAULT_INVALID_PAIR;
            }
            ImGui::EndChild();

            if (IsValidOutfit(m_wantEdit))
            {
                ImGui::SameLine();
                ImGui::BeginGroup();
                if (EditingPanel(m_wantEdit))
                {
                    m_wantEdit = DEFAULT_INVALID_PAIR;
                }
                ImGui::EndGroup();
            }
        }
        ImGui::End();
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
            +[&] {
                return m_outfitService.CreateOutfit(outfitNameBuf.data());
            };
        }

        if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
        {
            +[&] {
                return m_outfitService.CreateOutfitFromWorn(outfitNameBuf.data());
            };
        }
        ImGui::EndDisabled();

        if (ImGuiUtil::Button("$SosGui_Refresh{$SkyOutSys_MCM_OutfitList}"))
        {
            +[&] {
                return m_outfitService.GetOutfitList();
            };
        }

        //////////////////////////////////////////////////////////
        // Table Content
        auto       &outfits           = m_uiData.GetOutfitList();
        static bool onlyShowFavorites = false;
        if (ImGuiUtil::CheckBox("$SosGui_CheckBox_OnlyShowFavorites", &onlyShowFavorites))
        {
            outfits.OnlyFavoriteOutfits(onlyShowFavorites);
        }
        static size_t  selectedIdx = 0;
        constexpr auto flags = TableFlags().Borders().Sortable().SizingStretchProp().ScrollY().NoHostExtendX().flags;
        if (!ImGui::BeginTable("##OutfitLists", 3, flags))
        {
            return;
        }

        const auto activeOutfitOpt =                   //
            m_uiData                                   //
                .GetActorOutfitMap()                   //
                .TryGetOutfit(guiContext.editingActor) //
                .flat_map([&](auto &id) { return m_uiData.GetOutfitList().GetOutfit(id); });

        ImGuiListClipper clipper;
        // When current actor has active outfit: draw and freeze it on first row.
        const int offset = activeOutfitOpt.has_value() ? 1 : 0;
        clipper.Begin(outfits.size() + offset);
        clipper.IncludeItemByIndex(selectedIdx + offset);
        if (activeOutfitOpt.has_value())
        {
            ImGui::TableSetupScrollFreeze(1, 2);
            clipper.IncludeItemByIndex(0);
        }

        // clang-format off
        TableHeadersBuilder()
            .Column("##Number").NoSort().WidthFixed().NoHide()
            .Column("$SkyOutSys_MCM_OutfitList").DefaultSort().WidthStretch()
            .Column("##ActiveMark").NoSort().WidthFixed()
            .CommitHeadersRow();
        // clang-format on

        static bool ascend = true;
        ImGuiUtil::may_update_table_sort_dir(ascend);

        auto rowAction = [&](const auto &outfit, size_t index) {
            ImGuiUtil::PushIdGuard idGuard(index);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%zu", index + 1);

            bool activeOutfitRow = m_uiData.GetActorOutfitMap().IsActorOutfit(guiContext.editingActor, outfit.GetId());
            const auto &outfitName = outfit.GetName();
            if (ImGui::TableNextColumn()) // outfit name column
            {
                constexpr auto selectableFlags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                bool const     isSelected      = activeOutfitRow || selectedIdx == index;
                if (ImGui::Selectable(outfitName.data(), isSelected, selectableFlags))
                {
                    selectedIdx = index;
                    auto pair   = std::make_pair(outfit.GetId(), &outfit);
                    OnAcceptEditOutfit(pair);
                    m_wantEdit = pair;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
                if (OpenContextMenu(guiContext, outfit))
                {
                    m_click = std::make_pair(outfit.GetId(), &outfit);
                }
            }

            if (ImGui::TableNextColumn()) // active outfit hint column
            {
                if (activeOutfitRow)
                {
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

        if (activeOutfitOpt.has_value())
        {
            const auto             rank = m_uiData.GetOutfitList().Rank(activeOutfitOpt.value().GetId());
            ImGuiUtil::PushIdGuard idGuard(-1);
            rowAction(activeOutfitOpt.value(), rank);
        }
        while (clipper.Step())
        {
            int start = clipper.DisplayStart;
            int end   = clipper.DisplayEnd;
            outfits.for_each(ascend, start, end, rowAction);
        }
        ImGui::EndTable();
    }

    bool OutfitListTable::OpenContextMenu(GuiContext &guiContext, const SosUiOutfit &outfit)
    {
        if (!ImGui::BeginPopupContextItem("##OutfitListContextMenu"))
        {
            return false;
        }
        const auto &outfitName = outfit.GetName();

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
                    OnAcceptActiveOutfit(guiContext.editingActor, INVALID_OUTFIT_ID, "");
                }
            }
            if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOn"))
            {
                OnAcceptActiveOutfit(guiContext.editingActor, outfit.GetId(), outfitName);
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
            +[&] {
                return m_outfitService.DeleteOutfit(clicked.first, clicked.second->GetName());
            };
        }
        return justClosed;
    }

    void OutfitListTable::OnAcceptEditOutfit(const SosUiData::OutfitPair &wantEdit)
    {
        +[&] {
            return m_outfitService.GetOutfitArmors(wantEdit.first, wantEdit.second->GetName());
        };
        +[&] {
            return m_outfitService.GetSlotPolicy(wantEdit.first, wantEdit.second->GetName());
        };
        m_editPanel.ShowWindow(wantEdit.second->GetName());
    }

    void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, const OutfitId id,
                                               const std::string &outfitName)
    {
        +[&] {
            return m_outfitService.SetActorOutfit(editingActor, id, outfitName);
        };
    }
}
