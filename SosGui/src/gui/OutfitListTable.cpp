#include "gui/OutfitListTable.h"

#include "Translation.h"
#include "common/config.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/Table.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "util/ImGuiUtil.h"

#include <RE/A/Actor.h>
#include <array>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <format>
#include <functional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
void OutfitListTable::Refresh()
{
    OnRefreshOutfitList();
    m_editPanel.Refresh();
}

void OutfitListTable::Close()
{
    m_wantEdit = DEFAULT_INVALID_PAIR;
    m_outfitMultiSelection.Clear();
    m_outfitFilterInput.viewData.clear();
    m_editPanel.Close();
}

void OutfitListTable::OnSelectActor(const RE::Actor *actor)
{
    m_editPanel.OnSelectActor(actor, m_wantEdit.second);
}

void OutfitListTable::OnRefreshOutfitList()
{
    m_wantEdit = DEFAULT_INVALID_PAIR;
    m_outfitMultiSelection.Clear();
    m_onlyShowFavorites = false;
    m_outfitFilterInput.clear();
    m_outfitNameBuf[0] = '\0';
}

void OutfitListTable::OutfitDebounceInput::OnUpdate(const OutfitList &outfitList, const bool onlyFavorites)
{
    viewData.clear();
    if (onlyFavorites)
    {
        auto [it0, it1] = outfitList.FavoriteRankedIndex().equal_range(true);
        for (auto itBegin = it0; itBegin != it1; ++itBegin)
        {
            if (filter.PassFilter(itBegin->GetName().c_str()))
            {
                viewData.push_back(&*itBegin);
            }
        }
    }
    else
    {
        outfitList.for_each([&](const auto &outfit, size_t) {
            if (filter.PassFilter(outfit.GetName().c_str()))
            {
                viewData.push_back(&outfit);
            }
        });
    }

    dirty = false;
}

void OutfitListTable::Render(RE::Actor *editingActor)
{
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
    if (constexpr auto flags = ImGuiUtil::WindowFlags().flags;
        ImGui::Begin(Translation::Translate("$SkyOutSys_MCMHeader_OutfitList").c_str(), nullptr, flags))
    {
        DrawSidebar(editingActor);
    }
    // must out of BeginChild braces because when dock window, BeginChild will be return false;
    DrawDeletePopup();
    ImGui::End();

    if (IsValidOutfit(m_wantEdit))
    {
        ImGui::SetNextWindowSize({500, 300}, ImGuiCond_FirstUseEver);
        if (ImGui::Begin(windowName.c_str()))
        {
            ImGui::SameLine();
            ImGui::BeginGroup();
            m_editPanel.Render(m_wantEdit);
            ImGui::EndGroup();
        }
        ImGui::End();
    }
}

void OutfitListTable::DrawSidebar(RE::Actor *editingActor)
{
    //////////////////////////////////////////////////////////
    // Create Outfit widgets
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::InputText("##CreateNewInput", m_outfitNameBuf.data(), m_outfitNameBuf.size());

    ImGui::BeginDisabled(m_outfitNameBuf[0] == '\0');
    {
        if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
        {
            +[&] {
                return m_outfitService.CreateOutfit(m_outfitNameBuf.data());
            };
        }

        if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
        {
            +[&] {
                return m_outfitService.CreateOutfitFromWorn(m_outfitNameBuf.data());
            };
        }
    }
    ImGui::EndDisabled();

    auto &outfitList = m_uiData.GetOutfitList();
    if (ImGuiUtil::Button("$SosGui_Refresh"))
    {
        OnRefreshOutfitList();
        outfitList.clear();
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
    static size_t prevOutfitSize = 0;

    ImGui::PushItemWidth(-FLT_MIN);
    if (m_outfitFilterInput.Draw("##filter", "filter outfit") || prevOutfitSize != outfitList.size())
    {
        m_outfitFilterInput.OnUpdate(outfitList, m_onlyShowFavorites);
        prevOutfitSize = outfitList.size();
    }

    //////////////////////////////////////////////////////////
    // Table Content
    // clang-format off
    if (constexpr auto flags = TableFlags().Borders().Resizable().Hideable().Reorderable()
                                   .Sortable().SizingFixedFit().ScrollY().NoHostExtendX()
                                   .flags;
        !ImGui::BeginTable("##OutfitLists", 2, flags))
    {
        return;
    }
    // clang-format on

    const auto &actorOutfitMap  = m_uiData.GetActorOutfitMap();
    const auto  activeOutfitOpt = actorOutfitMap.TryGetOutfitId(editingActor).flat_map([&](auto &outfitId) {
        return outfitList.GetOutfitById(outfitId);
    });

    // When the current actor has active outfit: draw and freeze it on the first row.
    const auto activeOutfitId = activeOutfitOpt.map(GetOutfitId).value_or(INVALID_OUTFIT_ID);

    // clang-format off
   TableHeadersBuilder().Column("##Number").NoSort().WidthFixed().NoHide()
                        .Column("$SkyOutSys_MCM_OutfitList").DefaultSort()
                        .CommitHeadersRow();
    // clang-format on

    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);

    auto drawAction = [&](const auto &outfit, const size_t index) {
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
            static ImGuiID                                 editingInputId = 0;
            static std::array<char, MAX_OUTFIT_NAME_BYTES> renameBuf{};
            constexpr auto onRequestRename = [](const ImGuiID inputId, const std::string &outfitName) {
                editingInputId = inputId;
                strncpy_s(renameBuf.data(), renameBuf.size(), outfitName.c_str(), outfitName.size());
                ImGui::ActivateItemByID(inputId);
            };

            if (const auto thisInputId = ImGui::GetID("##editableOutfitNameId"); editingInputId == thisInputId)
            {
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputTextWithHint("##editableOutfitNameId", "rename outfit", renameBuf.data(), renameBuf.size());

                if (ImGui::IsItemDeactivated())
                {
                    if (renameBuf[0] != '\0' && strcmp(renameBuf.data(), outfit.GetName().c_str()) != 0)
                    {
                        log_info("Rename outfit {} to {}", outfit.GetName(), renameBuf.data());
                        +[&] {
                            return m_outfitService.RenameOutfit(outfit.GetId(), outfit.GetName(),
                                                                std::string(renameBuf.data()));
                        };
                    }
                    renameBuf[0]   = '\0';
                    editingInputId = 0;
                }
            }
            else
            {
                bool const isSelected = m_outfitMultiSelection.Contains(static_cast<ImGuiID>(index));
                ImGui::SetNextItemSelectionUserData(index);
                if (ImGui::Selectable(
                        outfit.GetName().c_str(), isSelected,
                        ImGuiUtil::SelectableFlag().AllowDoubleClick().AllowOverlap().SpanAllColumns().flags))
                {
                    auto pair = std::make_pair(outfit.GetId(), &outfit);
                    OnAcceptEditOutfit(m_wantEdit.second, pair);
                    m_wantEdit = pair;
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        onRequestRename(thisInputId, outfit.GetName());
                    }
                }
                bool acceptRename = false;
                OpenContextMenu(m_outfitMultiSelection.Size, editingActor, outfit, acceptRename);
                if (acceptRename)
                {
                    onRequestRename(thisInputId, outfit.GetName());
                }
            }
        }
    };

    DrawOutfits(m_outfitFilterInput.viewData, ascend, drawAction);
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
            for (const auto &outfit : outfitView | reverse | drop(clipper.DisplayStart) | take(itemCount))
            {
                drawAction(*outfit, index++);
            }
        }
    }
    PostDrawOutfits(m_outfitMultiSelection);
}

void OutfitListTable::OpenContextMenu(const uint32_t selectedItemCount, RE::Actor *editingActor,
                                      const SosUiOutfit &outfit, __out bool &acceptRename)
{
    if (!ImGui::BeginPopupContextItem("##OutfitListContextMenu"))
    {
        return;
    }
    const auto &outfitName = outfit.GetName();

    ImGui::Separator();
    const bool noEditingActor = editingActor == nullptr;

    if (selectedItemCount == 1)
    {
        ImGui::BeginDisabled(noEditingActor);
        {
            if (noEditingActor)
            {
                ImGuiUtil::Text("$SosGui_SelectHint{$Characters}");
            }
            const auto *actorName = noEditingActor ? "" : editingActor->GetName();
            ImGui::Text("%s", Translation::Translate("$SosGui_EditingActor", actorName).c_str());
            if (m_uiData.GetActorOutfitMap().IsActorOutfit(editingActor, outfit.GetId()))
            {
                if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOff"))
                {
                    OnAcceptActiveOutfit(editingActor, INVALID_OUTFIT_ID, "");
                }
            }
            if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOn"))
            {
                OnAcceptActiveOutfit(editingActor, outfit.GetId(), outfitName);
            }
            if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_Rename"))
            {
                acceptRename = true;
            }
        }
        ImGui::EndDisabled();
    }

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
        if (selectedItemCount == 1)
        {
            m_DeleteOutfitPopup.wanDeleteOutfit = &outfit;
            m_DeleteOutfitPopup.Open();
        }
        else
        {
            OnAcceptDeleteOutfits();
        }
    }
    ImGui::EndPopup();
}

void OutfitListTable::DrawDeletePopup()
{
    bool isConfirmDelete = false;
    if (m_DeleteOutfitPopup.Draw(isConfirmDelete); isConfirmDelete)
    {
        +[&] {
            return m_outfitService.DeleteOutfit(m_DeleteOutfitPopup.wanDeleteOutfit->GetId(),
                                                m_DeleteOutfitPopup.wanDeleteOutfit->GetName());
        };
        m_DeleteOutfitPopup.wanDeleteOutfit = nullptr;
    }
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

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, const OutfitId id,
                                           const std::string &outfitName) const
{
    +[&] {
        return m_outfitService.SetActorOutfit(editingActor, id, outfitName);
    };
    util::RefreshActorArmor(editingActor);
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptSetFavoriteOutfits(const bool toFavorite)
{
    const auto &outfitList = m_uiData.GetOutfitList();
    void       *it         = nullptr;
    ImGuiID     selectedRank; // must be name rank;

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

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptDeleteOutfits()
{
    const auto &outfitList = m_uiData.GetOutfitList();
    void       *it         = nullptr;
    ImGuiID     selectedRank; // must be name rank;

    while (m_outfitMultiSelection.GetNextSelectedItem(&it, &selectedRank))
    {
        if (auto opt = outfitList.GetOutfitByNameRank(selectedRank); opt.has_value())
        {
            +[&] {
                return m_outfitService.DeleteOutfit(opt.value().GetId(), opt.value().GetName());
            };
        }
    }
    m_outfitMultiSelection.Clear();
}
}