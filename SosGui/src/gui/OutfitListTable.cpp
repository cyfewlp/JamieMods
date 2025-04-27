#include "gui/OutfitListTable.h"

#include "GuiContext.h"
#include "Translation.h"
#include "common/config.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/Table.h"
#include "imgui.h"
#include "util/FunctionUtils.h"
#include "util/ImGuiUtil.h"

#include <RE/A/Actor.h>
#include <array>
#include <format>
#include <functional>
#include <string>
#include <utility>
#include <ranges>

namespace
LIBC_NAMESPACE_DECL
{
void OutfitListTable::Refresh()
{
    RefreshOutfitList();
    m_editPanel.Refresh();
}

void OutfitListTable::Close()
{
    m_click = DEFAULT_INVALID_PAIR;
    m_wantEdit = DEFAULT_INVALID_PAIR;
    m_outfitMultiSelection.Clear();
    m_outfitFilterInput.outfitView.clear();
    m_editPanel.Close();
}

void OutfitListTable::OnSelectActor(const RE::Actor *actor)
{
    m_editPanel.OnSelectActor(actor, m_wantEdit.second);
}

void OutfitListTable::RefreshOutfitList()
{
    m_click = DEFAULT_INVALID_PAIR;
    m_wantEdit = DEFAULT_INVALID_PAIR;
    m_outfitMultiSelection.Clear();
    m_onlyShowFavorites = false;
    m_outfitFilterInput.clear();
}

void OutfitListTable::outfit_debounce_input::onUpdate(const OutfitList &outfitList, const bool onlyFavorites)
{
    outfitView.clear();
    if (onlyFavorites)
    {
        auto [it0, it1] = outfitList.FavoriteRankedIndex().equal_range(true);
        for (auto itBegin = it0; itBegin != it1; ++itBegin)
        {
            if (filter.PassFilter(itBegin->GetName().c_str()))
            {
                outfitView.push_back(&*itBegin);
            }
        }
    }
    else
    {
        outfitList.for_each([&](const auto &outfit, size_t) {
            if (filter.PassFilter(outfit.GetName().c_str()))
            {
                outfitView.push_back(&outfit);
            }
        });
    }
}

void OutfitListTable::Render(GuiContext &guiContext)
{
    if (!m_uiData.GetOutfitList().HasOutfit(m_click.first))
    {
        m_click = DEFAULT_INVALID_PAIR;
    }
    if (!m_uiData.GetOutfitList().HasOutfit(m_wantEdit.first))
    {
        m_wantEdit = DEFAULT_INVALID_PAIR;
    }

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
            Sidebar(guiContext);
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

void OutfitListTable::Sidebar(GuiContext &guiContext)
{
    //////////////////////////////////////////////////////////
    // Create Outfit widgets
    static std::array<char, OUTFIT_NAME_MAX_BYTES> outfitNameBuf;

    ImGui::InputText("##CreateNewInput", outfitNameBuf.data(), outfitNameBuf.size());

    ImGui::BeginDisabled(outfitNameBuf[0] == '\0');
    {
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
    }
    ImGui::EndDisabled();

    if (ImGuiUtil::Button("$SosGui_Refresh"))
    {
        RefreshOutfitList();
        +[&] {
            return m_outfitService.GetOutfitList();
        };
        +[&] {
            return m_outfitService.GetAllFavoriteOutfits();
        };
    }

    ImGui::SameLine();
    if (ImGuiUtil::CheckBox("$SosGui_CheckBox_OnlyShowFavorites", &m_onlyShowFavorites))
    {
        m_outfitFilterInput.dirty = true;
    }

    auto &outfitList = m_uiData.GetOutfitList();
    static size_t prevOutfitSize = 0;

    if (m_outfitFilterInput.draw() || prevOutfitSize != outfitList.size())
    {
        m_outfitFilterInput.dirty = false;
        m_outfitFilterInput.onUpdate(outfitList, m_onlyShowFavorites);
        prevOutfitSize = outfitList.size();
    }

    //////////////////////////////////////////////////////////
    // Table Content
    if (constexpr auto flags = TableFlags().Borders().Resizable().Hideable().Reorderable()
                                           .Sortable().SizingFixedFit().ScrollY().NoHostExtendX()
                                           .flags;
        !ImGui::BeginTable("##OutfitLists", 3, flags))
    {
        return;
    }

    const auto &actorOutfitMap = m_uiData.GetActorOutfitMap();
    const auto activeOutfitOpt =
        actorOutfitMap.TryGetOutfitId(guiContext.editingActor).flat_map(GetOutfitFromId(outfitList));

    // When the current actor has active outfit: draw and freeze it on the first row.
    const auto activeOutfitId = activeOutfitOpt.map(GetOutfitId).value_or(INVALID_OUTFIT_ID);

    // clang-format off
   TableHeadersBuilder().Column("##Number").NoSort().WidthFixed().NoHide()
                        .Column("$SkyOutSys_MCM_OutfitList").DefaultSort()
                        .Column("##ActiveMark").NoSort().WidthFixed()
                        .CommitHeadersRow();
    // clang-format on

    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);

    auto drawAction = [&](const auto &outfit, size_t index) {
        ImGuiUtil::PushIdGuard idGuard(index);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); // number column
        ImGui::Text("%.4zu", index + 1);

        ImGui::TableNextColumn(); // outfit name column
        {
            if (outfit.IsFavorite())
            {
                ImGui::Text("\xe2\xad\x90");
                ImGui::SameLine();
            }
            bool const isSelected = m_outfitMultiSelection.Contains(static_cast<ImGuiID>(index));
            ImGui::SetNextItemSelectionUserData(index);
            if (constexpr auto selectableFlags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                ImGui::Selectable(outfit.GetName().c_str(), isSelected, selectableFlags))
            {
                auto pair = std::make_pair(outfit.GetId(), &outfit);
                OnAcceptEditOutfit(m_wantEdit.second, pair);
                m_wantEdit = pair;
            }
            if (OpenContextMenu(guiContext, outfit))
            {
                m_click = std::make_pair(outfit.GetId(), &outfit);
            }
        }

        if (ImGui::TableNextColumn()) // active outfit hint column
        {
            if (activeOutfitId == outfit.GetId())
            {
                ImGuiUtil::Text("$SkyOutSys_OutfitBrowser_ActiveMark");
            }
            else
            {
                ImGui::Text("");
            }
        }
    };

    if (activeOutfitOpt.has_value())
    {
        const auto rank = outfitList.GetNameRank(activeOutfitOpt.value().GetId());
        ImGuiUtil::PushIdGuard idGuard(-1);
        drawAction(activeOutfitOpt.value(), rank);
    }

    DrawOutfits(m_outfitFilterInput.outfitView, ascend, drawAction);
    ImGui::EndTable();
}

void OutfitListTable::PreDrawOutfits(ImGuiListClipper &clipper, MultiSelection &selection)
{
    auto *msIO = selection.NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid().Begin(clipper.ItemsCount);

    selection.ApplyRequests(msIO);
    if (msIO->RangeSrcItem != -1)
    {
        clipper.IncludeItemByIndex(static_cast<int>(msIO->RangeSrcItem));
    }
}

inline void OutfitListTable::PostDrawOutfits(MultiSelection &selection)
{
    selection.ApplyRequests(ImGui::EndMultiSelect());
}

void OutfitListTable::DrawOutfits(std::vector<const SosUiOutfit *> outfitView, const bool ascend,
                                  const OutfitDrawAction &drawAction)
{
    ImGuiListClipper clipper;
    clipper.Begin(outfitView.size());
    PreDrawOutfits(clipper, m_outfitMultiSelection);
    while (clipper.Step())
    {
        auto itemCount = clipper.DisplayEnd - clipper.DisplayStart;
        if (ascend)
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; index++)
            {
                drawAction(*outfitView[index], index);
            }
        }
        else
        {
            using namespace std::views;
            size_t index = 0;
            for (const auto &outfit : outfitView
                                      | reverse | drop(clipper.DisplayStart) | take(itemCount))
            {
                drawAction(*outfit, index++);
            }
        }
    }
    PostDrawOutfits(m_outfitMultiSelection);
}

bool OutfitListTable::OpenContextMenu(const GuiContext &guiContext, const SosUiOutfit &outfit)
{
    if (!ImGui::BeginPopupContextItem("##OutfitListContextMenu"))
    {
        return false;
    }
    const auto &outfitName = outfit.GetName();

    ImGui::Separator();
    const bool noEditingActor = guiContext.editingActor == nullptr;
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
    }
    ImGui::EndDisabled();
    ImGui::Separator();
    if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleFavoriteOn"))
    {
        OnAcceptSetFavoriteOutfits(true);
    }
    if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleFavoriteOff"))
    {
        OnAcceptSetFavoriteOutfits(false);
    }
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
    bool justClosed = m_DeleteOutfitPopup.Render(clicked.second->GetName(), isConfirmDelete);

    if (isConfirmDelete)
    {
        +[&] {
            return m_outfitService.DeleteOutfit(clicked.first, clicked.second->GetName());
        };
    }
    return justClosed;
}

void OutfitListTable::OnAcceptEditOutfit(const SosUiOutfit *lastEdit, const SosUiData::OutfitPair &wantEdit)
{
    +[&] {
        return m_outfitService.GetOutfitArmors(wantEdit.first, wantEdit.second->GetName());
    };
    +[&] {
        return m_outfitService.GetSlotPolicy(wantEdit.first, wantEdit.second->GetName());
    };
    m_editPanel.OnSelectOutfit(lastEdit, wantEdit.second);
}

void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, const OutfitId id, const std::string &outfitName)
{
    +[&] {
        return m_outfitService.SetActorOutfit(editingActor, id, outfitName);
    };
}

void OutfitListTable::OnAcceptSetFavoriteOutfits(bool toFavorite)
{
    const auto &outfitList = m_uiData.GetOutfitList();
    void *it = nullptr;
    ImGuiID selectedRank; // must be name rank;

    while (m_outfitMultiSelection.GetNextSelectedItem(&it, &selectedRank))
    {
        if (auto opt = outfitList.GetOutfitByNameRank(selectedRank); opt.has_value())
        {
            +[&] {
                return m_outfitService.SetOutfitIsFavorite(opt.value().GetId(), opt.value().GetName(), toFavorite);
            };
        }
    }
    m_outfitMultiSelection.Clear();
}
}