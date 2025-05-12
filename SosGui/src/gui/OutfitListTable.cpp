#include "gui/OutfitListTable.h"

#include "Translation.h"
#include "common/config.h"
#include "common/imgui/ImGuiScop.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/Config.h"
#include "gui/Table.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "util/ImGuiUtil.h"

#include <RE/A/Actor.h>
#include <array>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <functional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
void OutfitListTable::Cleanup()
{
    m_wantEdit = UNTITLED_OUTFIT;
    m_outfitMultiSelection.Clear();
    m_outfitFilterInput.clear();
}

void OutfitListTable::OnSelectActor(const RE::Actor *actor) const
{
    m_editPanel.OnSelectActor(actor, m_wantEdit);
}

bool OutfitListTable::OnModalPopupConfirmed(Popup::ModalPopup *modalPopup) const
{
    if (auto *deleteOutfitPopup = dynamic_cast<Popup::DeleteOutfitPopup *>(modalPopup); deleteOutfitPopup)
    {
        +[&] {
            return m_outfitService.DeleteOutfit(
                deleteOutfitPopup->wanDeleteOutfitId, deleteOutfitPopup->wanDeleteOutfitName
            );
        };
        return true;
    }

    if (auto *createOutfitPopup = dynamic_cast<CreateOutfitPopup *>(modalPopup); createOutfitPopup)
    {
        switch (createOutfitPopup->GetFlags())
        {
            case CreateOutfitPopup::Flags::CREATE_EMPTY:
                +[&] {
                    return m_outfitService.CreateOutfit(createOutfitPopup->GetOutfitName());
                };
                return true;
            case CreateOutfitPopup::Flags::CREATE_FROM_WORN:
                +[&] {
                    return m_outfitService.CreateOutfitFromWorn(createOutfitPopup->GetOutfitName());
                };
                return true;
            default:;
        }
    }
    return false;
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

void OutfitListTable::CreateOutfitPopup::DoDraw(SosUiData &, bool &confirmed)
{
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint(
        "##CreateNewInput", "$SosGui_Hint_CreateOutfit"_T.c_str(), m_outfitNameBuf.data(), m_outfitNameBuf.size()
    );
    ImGui::PopItemWidth();

    ImGuiScope::Disabled disabled(m_outfitNameBuf[0] == '\0');
    if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
    {
        ConfirmAndClose(confirmed);
        m_flags = Flags::CREATE_EMPTY;
    }

    if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
    {
        ConfirmAndClose(confirmed);
        m_flags = Flags::CREATE_FROM_WORN;
    }
}

void OutfitListTable::Draw(Context &context, RE::Actor *editingActor)
{
    if (!IsShowing())
    {
        return;
    }
    if (!m_uiData.GetOutfitList().HasOutfit(m_wantEdit.GetId()))
    {
        // TODO: only check on outfit list change?
        m_wantEdit = UNTITLED_OUTFIT;
    }

    ImGui::SetNextWindowPos(ImVec2(DEFAULT_OUTFIT_LIST_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(DEFAULT_WINDOW_WIDTH_SMALL, DEFAULT_WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
    if (constexpr auto flags = ImGuiUtil::WindowFlags().flags;
        ImGui::Begin(Translation::Translate("$SkyOutSys_MCMHeader_OutfitList").c_str(), &m_show, flags))
    {
        DoDraw(context, editingActor);
    }
    ImGui::End();
}

void OutfitListTable::DoDraw(Context &context, RE::Actor *editingActor)
{
    ImGuiScope::ItemWidth fltMinItemWidth(-FLT_MIN);

    auto &outfitList = m_uiData.GetOutfitList();
    if (ImGuiUtil::Button("$SosGui_Refresh"))
    {
        Cleanup();
        outfitList.clear();
        +[&] {
            return m_outfitService.GetOutfitList();
        };
        +[&] {
            return m_outfitService.GetAllFavoriteOutfits();
        };
    }

    ImGui::SameLine();
    if (ImGuiUtil::CheckBox("$SosGui_CheckBox_OnlyShowFavorites", &Config::SHOW_FAVORITE_OUTFITS))
    {
        m_outfitFilterInput.dirty = true;
    }
    static size_t prevOutfitSize = 0;

    if (m_outfitFilterInput.Draw("##filter", "$$SosGui_Hint_FilterOutfit"_T.c_str()) ||
        prevOutfitSize != outfitList.size())
    {
        m_outfitFilterInput.OnUpdate(outfitList, Config::SHOW_FAVORITE_OUTFITS);
        prevOutfitSize = outfitList.size();
    }

    //////////////////////////////////////////////////////////
    // Table Content
    ImGuiScope::Table outfitListTable(
        "##OutfitLists",
        2,
        TableFlags()
            .Borders()
            .Resizable()
            .Hideable()
            .Reorderable()
            .Sortable()
            .SizingStretchProp()
            .ScrollY()
            .NoHostExtendX()
            .ContextMenuInBody()
            .flags
    );
    if (!outfitListTable)
    {
        return;
    }

    const auto &actorOutfitMap  = m_uiData.GetActorOutfitMap();
    const auto  activeOutfitOpt = actorOutfitMap.TryGetOutfitId(editingActor).flat_map([&](auto &outfitId) {
        return outfitList.GetOutfitById(outfitId);
    });

    // When the current actor has active outfit: draw and freeze it on the first row.
    const auto activeOutfitId = activeOutfitOpt.map(GetOutfitId).value_or(INVALID_OUTFIT_ID);

    ImGui::TableSetupScrollFreeze(1, 1);
    // clang-format off
    TableHeadersBuilder().Column("##Number").WidthOrWeight(56 /*14 * 4*/).Flags(TableColumnFlags().NoSort().WidthFixed())
                         .Column("$SkyOutSys_MCM_OutfitList").Flags(TableColumnFlags().DefaultSort()).Setup();
    // clang-format on

    // Render our custom table header
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableNextColumn();
    ImGui::TableHeader(ImGui::TableGetColumnName(0));

    ImGui::TableNextColumn();
    ImGui::PushFontSize(Config::FONT_SIZE_TITLE_3);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    if (ImGui::Button("＋"))
    {
        context.popupList.push_back(std::make_unique<CreateOutfitPopup>());
    }
    ImGui::PopStyleVar();
    ImGui::SetItemTooltip("%s", "$SosGui_CreateOutfit"_T.c_str());
    ImGui::SameLine();
    ImGui::TableHeader(ImGui::TableGetColumnName(1));
    ImGui::PopFontSize();

    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);

    auto drawAction = [&](const auto &outfit, const size_t index) {
        ImGuiScope::PushId pushId(index);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); // number column
        ImGui::PushFontSize(Config::FONT_SIZE_TEXT_SMALL);
        ImGui::Text("%.4zu", index + 1);
        ImGui::PopFontSize();

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
                            return m_outfitService.RenameOutfit(
                                outfit.GetId(), outfit.GetName(), std::string(renameBuf.data())
                            );
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
                if (constexpr auto flags =
                        ImGuiUtil::SelectableFlag().AllowDoubleClick().AllowOverlap().SpanAllColumns().flags;
                    ImGui::Selectable(outfit.GetName().c_str(), isSelected, flags))
                {
                    const EditingOutfit currentEditing(outfit);
                    OnAcceptEditOutfit(m_wantEdit, currentEditing);
                    m_wantEdit = currentEditing;
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        onRequestRename(thisInputId, outfit.GetName());
                    }
                }
                bool acceptRename = false;
                OpenContextMenu(context, m_outfitMultiSelection.Size, editingActor, outfit, acceptRename);
                if (acceptRename)
                {
                    onRequestRename(thisInputId, outfit.GetName());
                }
            }
        }
    };

    DrawOutfits(m_outfitFilterInput.viewData, ascend, drawAction);
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

void OutfitListTable::DrawOutfits(
    std::vector<const SosUiOutfit *> outfitView, const bool ascend, const OutfitDrawAction &drawAction
)
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

void OutfitListTable::OpenContextMenu(
    Context &context, const uint32_t selectedItemCount, RE::Actor *editingActor, const SosUiOutfit &outfit,
    __out bool &acceptRename
)
{
    ImGuiScope::PopupContextItem popUp("##OutfitListContextMenu");
    if (!popUp)
    {
        return;
    }
    const auto &outfitName = outfit.GetName();

    ImGui::Separator();
    const bool noEditingActor = editingActor == nullptr;

    if (selectedItemCount == 1)
    {
        ImGuiScope::Disabled disabled(noEditingActor);
        if (noEditingActor)
        {
            ImGuiUtil::Text("$SosGui_Hint_Select{$Characters}");
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

    ImGui::Separator();
    if (ImGuiUtil::MenuItem("$SosGui_CreateOutfit"))
    {
        context.popupList.push_back(std::make_unique<CreateOutfitPopup>());
    }
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
            context.popupList.push_back(std::make_unique<Popup::DeleteOutfitPopup>(outfit.GetId(), outfit.GetName()));
        }
        else
        {
            OnAcceptDeleteOutfits();
        }
    }
}

void OutfitListTable::OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const
{
    +[&] {
        return m_outfitService.GetOutfitArmors(editingOutfit.GetId(), editingOutfit.GetName());
    };
    +[&] {
        return m_outfitService.GetSlotPolicy(editingOutfit.GetId(), editingOutfit.GetName());
    };
    m_editPanel.OnSelectOutfit(lastEdit, editingOutfit);
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, const OutfitId id, const std::string &outfitName)
    const
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