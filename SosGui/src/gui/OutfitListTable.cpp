#include "gui/OutfitListTable.h"

#include "Translation.h"
#include "data/OutfitList.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "gui/UiSettings.h"
#include "gui/icon.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "util/ImGuiUtil.h"

#include <array>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <d3d11.h>
#include <functional>
#include <ranges>
#include <string>
#include <tchar.h>
#include <utility>
#include <vector>

namespace SosGui
{
void OutfitListTable::Show()
{
    BaseGui::Show();
    OnAcceptEditOutfit(m_wantEdit, m_wantEdit); // trigger OutfitEditPanel#UpdateWindowTitle
}

void OutfitListTable::Focus()
{
    ImGui::SetWindowFocus("$SkyOutSys_MCMHeader_OutfitList"_T.c_str());
    BaseGui::Focus();
}

void OutfitListTable::OnRefresh()
{
    m_wantEdit = UNTITLED_OUTFIT;
    m_outfitMultiSelection.Clear();
    m_outfitFilterInput.Clear();
}

void OutfitListTable::Cleanup()
{
    m_wantEdit = UNTITLED_OUTFIT;
    m_outfitMultiSelection.Clear();
    m_outfitFilterInput.Clear();
}

bool OutfitListTable::OnModalPopupConfirmed(Popup::ModalPopup *modalPopup) const
{
    if (auto *deleteOutfitPopup = dynamic_cast<Popup::DeleteOutfitPopup *>(modalPopup); deleteOutfitPopup)
    {
        +[&] {
            return m_outfitService.DeleteOutfit(deleteOutfitPopup->wanDeleteOutfitId, deleteOutfitPopup->wanDeleteOutfitName);
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
    ImGui::InputTextWithHint("##CreateNewInput", "$SosGui_Hint_CreateOutfit"_T.c_str(), m_outfitNameBuf.data(), m_outfitNameBuf.size());
    ImGui::PopItemWidth();

    ImGui::BeginDisabled(m_outfitNameBuf[0] == '\0');
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
    ImGui::EndDisabled();
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
    if (ImGui::Begin(Translation::Translate("$SkyOutSys_MCMHeader_OutfitList").c_str(), &m_show, ImGuiEx::WindowFlags()))
    {
        DrawToolWidgets();
        DrawOutfitTable(context, editingActor);
    }
    ImGui::End();
}

void OutfitListTable::DrawToolWidgets()
{
    auto &outfitList = m_uiData.GetOutfitList();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    if (ImGuiUtil::IconButton(ICON_REFRESH_CW))
    {
        OnRefresh();
        outfitList.clear();
        +[&] {
            return m_outfitService.GetOutfitList();
        };
        +[&] {
            return m_outfitService.GetAllFavoriteOutfits();
        };
    }
    ImGui::PopStyleColor();
    ImGui::SetItemTooltip("%s", "$SosGui_Refresh{$SosGui_Outfit}"_T.c_str());

    auto *uiSetting = Settings::UiSettings::GetInstance();
    ImGui::SameLine();
    if (ImGuiUtil::CheckBox("$SosGui_CheckBox_OnlyShowFavorites", &uiSetting->showFavoriteOutfits))
    {
        m_outfitFilterInput.dirty = true;
    }
    static size_t prevOutfitSize = 0;

    ImGui::AlignTextToFramePadding();
    ImGuiUtil::IconButton(ICON_SEARCH);
    ImGui::SameLine(0.F, 0.F);

    ImGui::PushItemWidth(-FLT_MIN);
    if (m_outfitFilterInput.Draw("##filter", "$SosGui_Hint_FilterOutfit"_T.c_str()) || prevOutfitSize != outfitList.size())
    {
        m_outfitFilterInput.OnUpdate(outfitList, uiSetting->showFavoriteOutfits);
        prevOutfitSize = outfitList.size();
    }
    ImGui::PopItemWidth();
}

void OutfitListTable::DrawOutfitTable(Context &context, RE::Actor *editingActor)
{
    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_CellPadding>(ImVec2(10, 5));
    if (ImGui::BeginTable(
            "##OutfitLists",
            2,
            ImGuiEx::TableFlags()
                .Borders()
                .Resizable()
                .Hideable()
                .Reorderable()
                .Sortable()
                .SizingStretchProp()
                .ScrollY()
                .NoHostExtendX()
                .ContextMenuInBody()
        ))
    {
        DrawOutfitTableContent(context, editingActor);
        ImGui::EndTable();
    }
}

#define INVALID_INDEX SIZE_MAX

void OutfitListTable::DrawOutfitTableContent(Context &context, RE::Actor *editingActor)
{
    ImGui::TableSetupScrollFreeze(1, 1);
    ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoSort().WidthFixed(), 56);
    ImGui::TableSetupColumn("$SkyOutSys_MCM_OutfitList"_T.c_str(), ImGuiEx::TableColumnFlags().DefaultSort());
    ImGui::TableHeadersRow();

    // Render our custom table header
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableNextColumn();
    ImGui::TableHeader(ImGui::TableGetColumnName(0));

    ImGui::TableNextColumn();
    {
        ImGui::PushFont(nullptr, Settings::UiSettings::GetInstance()->Title3PxSize());
        if (ImGuiUtil::IconButton(ICON_FILE_PLUS_CORNER))
        {
            context.popupList.push_back(std::make_unique<CreateOutfitPopup>());
        }
        ImGui::SetItemTooltip("%s", "$SosGui_CreateOutfit"_T.c_str());
        ImGui::SameLine();
        ImGui::TableHeader(ImGui::TableGetColumnName(1));
        ImGui::PopFont();
    }

    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);

    size_t clickedFavoriteId = INVALID_INDEX;
    auto   drawOutfitEntry   = [&](const auto &outfit, const size_t index) {
        ImGui::PushID(index);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); // number column
        {
            const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({5.0F, 5.0F}).Color<ImGuiCol_Button>(ImVec4());
            bool       clicked    = false;
            if (outfit->IsFavorite())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(ImColor(234, 51, 35)));
                clicked = ImGuiUtil::IconButton(ICON_HEART);
                ImGui::PopStyleColor();
                ImGui::SetItemTooltip("%s", "$SkyOutSys_OContext_ToggleFavoriteOff"_T.c_str());
            }
            else
            {
                clicked = ImGuiUtil::IconButton(ICON_HEART_OFF);
                ImGui::SetItemTooltip("%s", "$SkyOutSys_OContext_ToggleFavoriteOn"_T.c_str());
            }
            if (clicked)
            {
                clickedFavoriteId = index;
            }

            ImGui::SameLine(0, 10);
            ImGui::Text("%zu", index + 1);
        }

        ImGui::TableNextColumn(); // outfit name column
        {
            static ImGuiID                                 editingInputId = 0;
            static std::array<char, MAX_OUTFIT_NAME_BYTES> renameBuf{};
            constexpr auto                                 onRequestRename = [](const ImGuiID inputId, const std::string &outfitName) {
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
                    if (renameBuf[0] != '\0' && strcmp(renameBuf.data(), outfit->GetName().c_str()) != 0)
                    {
                        logger::info("Rename outfit {} to {}", outfit->GetName(), renameBuf.data());
                        +[&] {
                            return m_outfitService.RenameOutfit(outfit->GetId(), outfit->GetName(), std::string(renameBuf.data()));
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
                if (constexpr auto flags = ImGuiEx::SelectableFlags().AllowDoubleClick().AllowOverlap().SpanAllColumns();
                    ImGui::Selectable(outfit->GetName().c_str(), isSelected, flags))
                {
                    const EditingOutfit currentEditing(outfit);
                    OnAcceptEditOutfit(m_wantEdit, currentEditing);
                    m_wantEdit = currentEditing;
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        onRequestRename(thisInputId, outfit->GetName());
                    }
                }
                bool acceptRename = false;
                OpenContextMenu(context, m_outfitMultiSelection.Size, editingActor, outfit, acceptRename);
                if (acceptRename)
                {
                    onRequestRename(thisInputId, outfit->GetName());
                }
            }
        }
        ImGui::PopID();
    };

    auto &viewData = m_outfitFilterInput.viewData;

    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({3.0F, 3.0F});
    DrawOutfitTableContent(viewData, ascend, drawOutfitEntry);

    if (clickedFavoriteId != INVALID_INDEX)
    {
        OnToggleFavorite(viewData, clickedFavoriteId);
    }
}

inline void OutfitListTable::OnToggleFavorite(std::vector<const SosUiOutfit *> &viewData, size_t index) const
{
    // Erase viewData directly. We assume that the outfit exists in the container.
    const auto outfit = viewData[index];
    if (Settings::UiSettings::GetInstance()->showFavoriteOutfits)
    {
        viewData.erase(viewData.begin() + index);
    }
    +[&] {
        return m_outfitService.SetOutfitIsFavorite(outfit->GetId(), outfit->GetName(), !outfit->IsFavorite());
    };
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

void OutfitListTable::DrawOutfitTableContent(std::vector<const SosUiOutfit *> outfitView, const bool ascend, const DrawOutfitEntry &drawOutfitEntry)
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
                drawOutfitEntry(outfitView[index], index);
            }
        }
        else
        {
            using namespace std::views;
            size_t index = static_cast<size_t>(clipper.DisplayStart);
            for (const auto &outfit : outfitView | reverse | drop(clipper.DisplayStart) | take(itemCount))
            {
                drawOutfitEntry(outfit, index);
                index++;
            }
        }
    }
    PostDrawOutfits(m_outfitMultiSelection);
}

void OutfitListTable::OpenContextMenu(
    Context &context, const uint32_t selectedItemCount, RE::Actor *editingActor, const SosUiOutfit *outfit, __out bool &acceptRename
)
{
    if (!ImGui::BeginPopupContextItem("##OutfitListContextMenu"))
    {
        ImGui::EndPopup();
        return;
    }
    const auto &outfitName = outfit->GetName();

    ImGui::Separator();
    const bool noEditingActor = editingActor == nullptr;

    if (selectedItemCount == 1)
    {
        ImGui::BeginDisabled(noEditingActor);
        if (noEditingActor)
        {
            ImGuiUtil::Text("$SosGui_Hint_Select{$Characters}");
        }
        const auto *actorName = noEditingActor ? "" : editingActor->GetName();
        ImGui::Text("%s", Translation::Translate("$SosGui_EditingActor", actorName).c_str());
        if (m_uiData.GetActorOutfitMap().IsActorOutfit(editingActor, outfit->GetId()))
        {
            if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOff"))
            {
                OnAcceptActiveOutfit(editingActor, INVALID_OUTFIT_ID, "");
            }
        }
        if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOn"))
        {
            OnAcceptActiveOutfit(editingActor, outfit->GetId(), outfitName);
        }
        if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_Rename"))
        {
            acceptRename = true;
        }
        ImGui::EndDisabled();
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
            context.popupList.push_back(std::make_unique<Popup::DeleteOutfitPopup>(outfit->GetId(), outfit->GetName()));
        }
        else
        {
            OnAcceptDeleteOutfits();
        }
    }
    ImGui::EndPopup();
}

void OutfitListTable::OnAcceptEditOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editingOutfit) const
{
    if (!editingOutfit.IsUntitled())
    {
        +[&] {
            return m_outfitService.GetOutfitArmors(editingOutfit.GetId(), editingOutfit.GetName());
        };
        +[&] {
            return m_outfitService.GetSlotPolicy(editingOutfit.GetId(), editingOutfit.GetName());
        };
    }

    m_editPanel.OnSelectOutfit(lastEdit, editingOutfit);
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, const OutfitId id, const std::string &outfitName) const
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
} // namespace SosGui
