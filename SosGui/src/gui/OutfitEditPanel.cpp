#include "gui/OutfitEditPanel.h"

#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "common/imgui/ImGuiFlags.h"
#include "common/imgui/ImGuiScope.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/Popup.h"
#include "gui/Table.h"
#include "gui/UiSetting.h"
#include "gui/icon.h"
#include "gui/widgets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "util/ImGuiUtil.h"
#include "util/utils.h"

#include <RE/B/BGSBipedObjectForm.h>
#include <RE/B/BipedObjects.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <array>
#include <cassert>
#include <expected>
#include <format>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
void OutfitEditPanel::EditContext::Clear()
{
    ShowAllSlotOutfitArmors = false;
    dirty                   = true;
}

void OutfitEditPanel::OnRefresh()
{
    m_editContext.Clear();
    m_armorView.on_refresh();
}

void OutfitEditPanel::Cleanup()
{
    m_editContext.Clear();
    m_armorView.clear();
}

void OutfitEditPanel::OnSelectActor(const RE::Actor *, const EditingOutfit &editingOutfit)
{
    m_armorView.update_view_data(GetGenerator(), editingOutfit.GetSourceOutfit());
}

void OutfitEditPanel::OnSelectOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editing)
{
    UpdateWindowTitle(editing.GetName());
    if (dynamic_cast<BasicArmorGenerator *>(GetGenerator()) != nullptr)
    {
        if (lastEdit.GetId() != editing.GetId())
        {
            m_armorView.add_armors_in_outfit(m_uiData, lastEdit.GetSourceOutfit());
            m_armorView.remove_armors_in_outfit(editing.GetSourceOutfit());
            m_armorView.multiSelection.Clear();
        }
    }
    else
    {
        m_armorView.update_view_data(GetGenerator(), editing.GetSourceOutfit());
    }
}

void OutfitEditPanel::PushError(SosUiData &uiData, const Error error)
{
    switch (error)
    {
        case Error::delete_armor_from_unknown_outfit_id:
            uiData.PushErrorMessage("Can't delete a invalid or has been removed outfit.");
            break;
        case Error::add_armor_to_unknown_outfit_id:
            uiData.PushErrorMessage("Can't add armor to a invalid or has been removed outfit.");
            break;
    }
}

void OutfitEditPanel::Draw(Context &context, const EditingOutfit &editingOutfit)
{
    if (!IsShowing())
    {
        return;
    }
    const std::string windowName = std::format("Editing Outfit: {}###OutfitEditor", editingOutfit.GetName());
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_OUTFIT_EDIT_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(windowName.c_str(), &m_show))
    {
        DoDraw(context, editingOutfit);
    }
    ImGui::End();
}

void OutfitEditPanel::DoDraw(Context &context, const EditingOutfit &editingOutfit)
{
    if (m_editContext.dirty)
    {
        m_editContext.dirty = false;
        m_armorView.update_view_data(GetGenerator(), editingOutfit.GetSourceOutfit());
    }
    if (ImGui::BeginChild("##OutfitEditPanelSideBar", {200, 0}, ImGuiUtil::ChildFlag().Borders().ResizeX().flags))
    {
        DrawSideBar(editingOutfit.GetSourceOutfit());
    }
    ImGui::EndChild();

    {
        ImGui::SameLine();
        ImGuiScope::Group group;
        DrawOutfitPanel(context, editingOutfit);
        DrawArmorGeneratorTabBar(editingOutfit.GetSourceOutfit());
        DrawArmorViewFilter(editingOutfit.GetSourceOutfit());
        DrawArmorView(context, editingOutfit);
    }
}

bool OutfitEditPanel::OnModalPopupConfirmed(Popup::ModalPopup *modalPopup)
{
    if (auto *modifyRequest = dynamic_cast<OutfitModifyRequest *>(modalPopup); modifyRequest)
    {
        modifyRequest->OnConfirm(this);
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Outfit Panel
////////////////////////////////////////////////////////////////////////////////////////////////

void OutfitEditPanel::DrawOutfitPanel(Context &context, const EditingOutfit &editingOutfit)
{
    ImGuiUtil::TextScale(editingOutfit.GetName().c_str(), Setting::UiSetting::FONT_SIZE_TITLE_3);
    if (auto tabBar = ImGuiScope::TabBar("##OutfitTabBarView"))
    {
        if (auto tabItem = ImGuiScope::TabItem("$Armor"_T.c_str(), nullptr, ImGuiTabItemFlags_Leading))
        {
            DrawOutfitArmors(context, editingOutfit);
        }
    }
}

void OutfitEditPanel::DrawSideBar(const SosUiOutfit *editingOutfit)
{
    ImGuiUtil::TextScale("$SosGui_ModList", Setting::UiSetting::FONT_SIZE_TITLE_3);
    static int maxChildItemCount = 10;
    const auto itemHeight        = ImGui::GetTextLineHeight();
    float      childHeight       = (itemHeight + ImGui::GetStyle().ItemInnerSpacing.y) * maxChildItemCount;

    if (ImGui::BeginChild("##ModNameListChild", {0, childHeight}, ImGuiUtil::ChildFlag().Borders().ResizeY()))
    {
        DrawArmorViewModNameFilterer(editingOutfit);
    }
    ImGui::EndChild();

    ImGuiUtil::TextScale("$SosGui_BodySlots", Setting::UiSetting::FONT_SIZE_TITLE_3);
    if (ImGui::BeginChild("#SlotFilterChild", {0, childHeight}, ImGuiUtil::ChildFlag().Borders().ResizeY()))
    {
        DrawArmorViewSlotFilterer(editingOutfit);
    }
    ImGui::EndChild();
}

void OutfitEditPanel::UpdateWindowTitle(const std::string &outfitName)
{
    m_windowTitle = Translation::Translate("$SosGui_WindowName_EditingOutfit");
    if (const auto pos = m_windowTitle.find("{}"); pos != std::string::npos)
    {
        m_windowTitle.replace(pos, 2, outfitName);
    }
}

void OutfitEditPanel::DrawOutfitArmors(Context &context, const EditingOutfit &editingOutfit)
{
    if (editingOutfit.IsUntitled())
    {
        ImGuiUtil::TextScale("$SosGui_Hint_Select{$SosGui_Outfit}", Setting::UiSetting::FONT_SIZE_TITLE_3);
        return;
    }
    if (editingOutfit.IsEmpty())
    {
        ImGuiUtil::TextScale("$SosGui_Hint_Empty{$ARMOR}", Setting::UiSetting::FONT_SIZE_TITLE_3);
        return;
    }

    ImGui::Checkbox("$SosGui_CheckBox_SlotFilter"_T.c_str(), &m_editContext.ShowAllSlotOutfitArmors);
    ImGui::SameLine();
    if (ImGui::Button("$SkyOutSys_OEdit_SlotPolicyHelp"_T.c_str()))
    {
        context.popupList.push_back(std::make_unique<SlotPolicyHelp>());
    }

    static int selectedIdx = -1;
    if (auto table =
            ImGuiScope::Table("##OutfitArmors", 5, ImGuiUtil::TableFlags().Resizable().SizingStretchProp().flags))
    {
        // clang-format off
        TableHeadersBuilder().Column("##Number").Flags(ImGuiUtil::TableColumnFlags().NoHide())
            .Column("$SosGui_TableHeader_Slot").Flags(ImGuiUtil::TableColumnFlags().NoSort())
            .Column("$ARMOR").Flags(ImGuiUtil::TableColumnFlags().NoSort())
            .Column("$SkyOutSys_OEdit_OutfitSettings_Header").Flags(ImGuiUtil::TableColumnFlags().WidthFixed().NoSort())
            .Column("$Delete").Flags(ImGuiUtil::TableColumnFlags().WidthFixed().NoSort())
            .CommitHeadersRow();
        // clang-format on

        for (uint32_t slotIdx = 0; slotIdx < RE::BIPED_OBJECT::kEditorTotal; ++slotIdx)
        {
            const auto *armor = editingOutfit.GetArmorAt(slotIdx);
            if (!m_editContext.ShowAllSlotOutfitArmors && armor == nullptr)
            {
                continue;
            }

            ImGuiScope::PushId pushId(slotIdx);

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%d", slotIdx + 1);

            if (ImGui::TableNextColumn()) // slot name column
            {
                const bool isSelected = static_cast<uint32_t>(selectedIdx) == slotIdx;
                auto       name       = std::format("$SkyOutSys_BodySlot{}", slotIdx + SOS_SLOT_OFFSET);
                if (ImGuiUtil::Selectable(
                        name, isSelected, ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags
                    ))
                {
                    selectedIdx = isSelected ? -1 : slotIdx;
                }
            }
            if (armor != nullptr)
            {
                HighlightConflictSlot(ToSlot(slotIdx));
            }

            ImGui::TableNextColumn(); // armor name column
            ImGui::Text("%s", armor == nullptr ? "" : armor->GetName());

            ImGui::TableNextColumn(); // slot policy combo column
            SlotPolicyCombo(editingOutfit, slotIdx);

            ImGui::TableNextColumn(); // action column
            {
                ImGuiScope::Disabled disabled(armor == nullptr);
                if (ImGui::Button("$Delete"_T.c_str()))
                {
                    context.popupList.emplace_back(std::make_unique<DeleteRequest>(editingOutfit.GetId(), armor));
                }
            }
        }
    }
}

void OutfitEditPanel::HighlightConflictSlot(const Slot slot) const
{
    if (m_armorView.multiSelection.IsSelectSlot(slot))
    {
        auto *drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(
            ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 0, 0, 255), 0, ImDrawFlags_None, 2.0F
        );
    }
}

void OutfitEditPanel::SlotPolicyCombo(const EditingOutfit &editingOutfit, const uint32_t &slotIdx) const
{
    auto       &policyNameKey = editingOutfit.GetSlotPolicies().at(slotIdx);
    std::string policyName;

    if (auto combo = ImGuiScope::Combo(
            "##SlotPolicy", Translation::Translate(policyNameKey).c_str(), ImGuiComboFlags_WidthFitPreview
        ))
    {
        for (const auto &policy :
             {SlotPolicy::Inherit, SlotPolicy::Passthrough, SlotPolicy::RequireEquipped, SlotPolicy::AlwaysUseOutfit})
        {
            policyName = SlotPolicyToUiString(policy);
            if (ImGui::Selectable(policyName.c_str(), false))
            {
                if (!editingOutfit.IsUntitled())
                {
                    +[&] {
                        return m_outfitService.SetSlotPolicy(
                            editingOutfit.GetId(), editingOutfit.GetName(), slotIdx, policy
                        );
                    };
                }
            }
            ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
        }
    }
}

inline auto OutfitEditPanel::get_slot_name_key(const uint32_t slotPos) -> std::string
{
    return std::format("$SkyOutSys_BodySlot{}", slotPos + SOS_SLOT_OFFSET);
}

void OutfitEditPanel::DrawArmorGeneratorTabBar(const SosUiOutfit *editingOutfit)
{
    using namespace ImGuiUtil;
    ImGui::Separator();
    TextScale("$SosGui_ArmorGenerator", Setting::UiSetting::FONT_SIZE_TITLE_3);
    if (auto tabBarW =
            ImGuiScope::TabBar("ArmorGeneratorTabBar", TabBarFlags().DrawSelectedOverline().Reorderable().flags))
    {
        const auto *tabBar          = ImGui::GetCurrentTabBar();
        const auto  nextTabId       = tabBar->NextSelectedTabId;
        const auto  selectedId      = tabBar->SelectedTabId;
        auto        isTabItemAppear = [&] {
            const ImGuiTabItem *tabItem = ImGui::TabBarGetCurrentTab(ImGui::GetCurrentTabBar());
            return selectedId == 0 || (selectedId != nextTabId && tabItem->ID == nextTabId);
        };
        auto  *player         = RE::PlayerCharacter::GetSingleton();
        auto *&selectedActor  = m_armorGeneratorTabBar.selectedActor;
        auto  &armorGenerator = m_armorGeneratorTabBar.generator;
        if (auto tabItem = ImGuiScope::TabItem("$SosGui_ArmorGenerator_NearObjectInventory"_T))
        {
            auto *generator = dynamic_cast<NearObjectsInventoryArmorGenerator *>(armorGenerator.get());
            if (!generator || isTabItemAppear())
            {
                generator = new NearObjectsInventoryArmorGenerator;
                generator->Update();
                armorGenerator.reset(generator);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }

            const auto &nearObjects = generator->NearObjects();
            auto       *wantVisit   = nearObjects[generator->WantVisitIndex()];

            if (auto combo = ImGuiScope::Combo(
                    "$SosGui_NearObjects"_T, wantVisit->GetDisplayFullName(), ImGuiComboFlags_WidthFitPreview
                ))
            {
                for (size_t index = 0; index < nearObjects.size(); ++index)
                {
                    ImGuiScope::PushId pushId(index);
                    if (ImGui::Selectable(nearObjects[index]->GetName(), generator->WantVisitIndex() == index))
                    {
                        generator->SetWantVisitIndex(index);
                        m_armorView.reset_view(GetGenerator(), editingOutfit);
                    }
                }
            }
        }

        if (auto tabItem = ImGuiScope::TabItem("$SosGui_ArmorGenerator_NearNpcCarried"_T))
        {
            if (isTabItemAppear())
            {
                armorGenerator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            Text("$SosGui_Actor_ArmorSource");
            ImGui::SameLine();
            if (widgets::DrawNearActorsCombo(m_uiData.GetNearActors(), &selectedActor, player))
            {
                armorGenerator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
        }

        if (auto tabItem = ImGuiScope::TabItem("$SosGui_ArmorGenerator_FormID"_T))
        {
            if (isTabItemAppear())
            {
                armorGenerator = nullptr;
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::Text("0x");
            ImGui::SameLine();
            static std::array<char, 9> formIdBuf;

            if (ImGui::InputText(
                    "##FormIdInput",
                    formIdBuf.data(),
                    formIdBuf.size(),
                    InputTextFlags().CharsUppercase().CharsHexadecimal()
                ))
            {
                m_armorView.clearViewData();
                char          *pEnd{};
                const uint32_t result = strtoul(formIdBuf.data(), &pEnd, 16);
                if (pEnd != formIdBuf.data())
                {
                    armorGenerator = std::make_unique<FormIdArmorGenerator>(result);
                    m_armorView.reset_view(GetGenerator(), editingOutfit);
                }
            }
        }

        if (auto tabItem = ImGuiScope::TabItem("$SosGui_ArmorGenerator_AllArmors"_T))
        {
            if (isTabItemAppear())
            {
                armorGenerator = std::make_unique<BasicArmorGenerator>();
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
        }
    }
}

void OutfitEditPanel::DrawArmorViewFilter(const SosUiOutfit *editingOutfit)
{
    {
        auto framePadding = ImGuiScope::StyleVar::FramePadding({5.0f, 5.0f});
        auto buttonColor  = ImGuiScope::StyleColor::Button(ImVec4(0, 0, 0, 0));
        if (ImGui::Button(NF_OCT_FILTER))
        {
            ImGui::OpenPopup("Filters");
        }
        ImGui::SetItemTooltip("%s", "$SosGui_Filterers"_T.c_str());
    }
    ImGui::SetNextWindowPos({ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y});
    if (auto popup = ImGuiScope::Popup("Filters"))
    {
        bool needUpdate =
            ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &m_armorView.armorFilter.mustPlayable);
        needUpdate |= ImGuiUtil::CheckBox(
            "$SosGui_CheckBox_TemplateArmor", &Setting::UiSetting::GetInstance()->includeTemplateArmor
        );
        if (needUpdate)
        {
            m_armorView.update_view_data(GetGenerator(), editingOutfit);
        }
    }

    ImGui::SameLine(0, 20);
    if (m_armorView.armorFilter.Draw())
    {
        m_armorView.armorFilter.dirty = false;
        m_armorView.update_view_data(GetGenerator(), editingOutfit);
    }
}

void OutfitEditPanel::DrawArmorView(Context &context, const EditingOutfit &editingOutfit)
{
    if (m_armorView.ViewData().empty())
    {
        return;
    }
    constexpr auto flags = ImGuiUtil::TableFlags()
                               .RowBg()
                               .BordersInnerH()
                               .ScrollY()
                               .Resizable()
                               .SizingStretchProp()
                               .Sortable()
                               .Hideable()
                               .Reorderable()
                               .flags;
    auto framePadding = ImGuiScope::StyleVar::FramePadding(Setting::UiSetting::TABLE_ROW_PADDING);
    if (const auto armorViewTable = ImGuiScope::Table("##ArmorCandidates", 6, flags))
    {
        DrawArmorViewContent(context, editingOutfit, m_armorView.ViewData());
    }
}

void OutfitEditPanel::DrawArmorViewContent(
    Context &context, const EditingOutfit &editingOutfit, const std::vector<ArmorView::RankedArmor> &viewData
)
{
    using namespace ImGuiUtil;
    ImGui::TableSetupScrollFreeze(1, 1);
    // clang-format off
    TableHeadersBuilder().Column("##Number").Flags(TableColumnFlags().NoSort().WidthFixed())
        .Column("$ARMOR").Flags(TableColumnFlags().DefaultSort().NoHide())
        .Column("FormID").Flags(TableColumnFlags().NoSort())
        .Column("ModName").Flags(TableColumnFlags().NoSort())
        .Column("Playable").Flags(TableColumnFlags().NoSort())
        .Column("$Add").Flags(TableColumnFlags().WidthFixed().NoSort());
    // clang-format on
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (int colIndex = 0; colIndex < 6; ++colIndex)
    {
        ImGui::TableSetColumnIndex(colIndex);
        ImGuiScope::FontSize fontSize(Setting::UiSetting::FONT_SIZE_TITLE_3);
        ImGui::TableHeader(ImGui::TableGetColumnName(colIndex));
    }

    std::function<void()> onRequireAddArmor = [] {};

    auto drawArmorEntry = [&](const Armor *armor, const size_t index) {
        ImGuiScope::PushId pushId(index);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); // number column
        {
            const bool isSelected = m_armorView.multiSelection.UpdateSelected(armor, index);
            ImGui::SetNextItemSelectionUserData(index);
            ImGui::Selectable(
                std::format("{}", index + 1).c_str(), isSelected, SelectableFlag().AllowOverlap().SpanAllColumns().flags
            );

            if (auto popup = ImGuiScope::PopupContextItem("Context"))
            {
                ImGuiScope::Disabled disabled(editingOutfit.IsUntitled());
                if (MenuItem("$SosGui_ContextMenu_AddAllArmor"))
                {
                    if (m_armorView.multiSelection.Size == 1)
                    {
                        onRequireAddArmor = [&, armor] {
                            OnAcceptAddArmorToOutfit(context, editingOutfit, armor);
                        };
                    }
                    else if (m_armorView.multiSelection.Size > 1)
                    {
                        context.popupList.emplace_back(std::make_unique<BatchAddArmorsRequest>(editingOutfit.GetId()));
                    }
                }
            }
        }

        if (ImGui::TableNextColumn()) // armor name column
        {
            ImGui::Text("%s", armor->GetName());
        }

        if (ImGui::TableNextColumn()) // formId column
        {
            ImGui::Text("%s", std::format("{:#010X}", armor->GetFormID()).c_str());
        }

        if (ImGui::TableNextColumn()) // mod name column
        {
            ImGui::Text("%s", util::GetFormModFileName(armor).data());
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                ImGui::SetItemTooltip("%s", util::GetFormModFileName(armor).data());
            }
        }

        if (ImGui::TableNextColumn()) // column playable
        {
            ImGui::Text("%s", IsArmorNonPlayable(armor) ? "\xe2\x9d\x8c" : "\xe2\x9c\x85");
        }

        ImGuiScope::Disabled disabled(editingOutfit.IsUntitled());
        if (ImGui::TableNextColumn() && Button("$Add")) // column Action
        {
            onRequireAddArmor = [&, armor] {
                OnAcceptAddArmorToOutfit(context, editingOutfit, armor);
            };
        }
    };

    DrawArmorViewTableContent(viewData, drawArmorEntry);
    if (!editingOutfit.IsUntitled())
    {
        onRequireAddArmor();
    }
}

void OutfitEditPanel::DrawArmorViewTableContent(
    const std::vector<ArmorView::RankedArmor> &viewData, const DrawArmorEntry &drawArmorEntry
)
{
    static bool ascend = true;
    ImGuiUtil::may_update_table_sort_dir(ascend);
    ImGuiListClipper clipper;
    // clang-format off
    auto *msIO = m_armorView.multiSelection
                 .NoSelectAll().BoxSelect1d().ClearOnEscape().ClearOnClickVoid()
                 .Begin(viewData.size());
    clipper.Begin(viewData.size());
    m_armorView.multiSelection.ApplyRequests(msIO);
    if (msIO->RangeSrcItem != -1)
        clipper.IncludeItemByIndex(static_cast<int>(msIO->RangeSrcItem));

    // clang-format on
    while (clipper.Step())
    {
        if (ascend)
        {
            for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
            {
                drawArmorEntry(viewData.at(index), index);
            }
        }
        else
        {
            using namespace std::views;
            size_t index = static_cast<size_t>(clipper.DisplayStart);
            for (int         count = clipper.DisplayEnd - clipper.DisplayStart;
                 const auto &armor : viewData | reverse | drop(clipper.DisplayStart) | take(count))
            {
                drawArmorEntry(armor, index++);
            }
        }
    }
    m_armorView.multiSelection.ApplyRequests(ImGui::EndMultiSelect());
}

void OutfitEditPanel::DrawArmorViewModNameFilterer(const SosUiOutfit *editingOutfit)
{
    bool needResetView = false;
    for (auto &modState : m_armorView.modFilterer.passModList)
    {
        if (auto &modName = modState.first; m_armorView.modRefCounter.contains(modName))
        {
            auto label = std::format("({}) {}", m_armorView.modRefCounter[modName], modName);

            bool tIsChecked = modState.second;
            if (ImGui::Checkbox(label.c_str(), &tIsChecked))
            {
                std::pair newState(modName, tIsChecked);
                modState.swap(newState);
                needResetView = true;
            }
        }
    }
    if (needResetView)
    {
        m_armorView.update_view_data(GetGenerator(), editingOutfit);
    }
}

void OutfitEditPanel::DrawArmorViewSlotFilterer(const SosUiOutfit *editing)
{
    const std::string name =
        std::format("All({}/{})##AllSlot", m_armorView.ViewData().size(), m_armorView.availableArmorCount);
    if (ImGui::Checkbox(name.c_str(), &m_armorView.checkAllSlot))
    {
        if (!m_armorView.checkAllSlot)
        {
            if (m_armorView.slotFiltererSelected == 0)
            {
                m_armorView.clearViewData();
            }
            else
            {
                auto slotValue = m_armorView.slotFiltererSelected.to_ulong();
                m_armorView.remove_armors_has_slot(static_cast<Slot>(slotValue), static_cast<Slot>(~slotValue));
            }
        }
        else
        {
            m_armorView.update_view_data(GetGenerator(), editing);
        }
    }

    for (uint8_t idx = 0; idx < RE::BIPED_OBJECT::kEditorTotal; ++idx)
    {
        ImGuiScope::PushId pushId(idx);

        auto slotLabel =
            std::format("({}) {}", m_armorView.slotCounter[idx], Translation::Translate(get_slot_name_key(idx)));
        bool checked = m_armorView.slotFiltererSelected.test(idx);
        if (ImGui::Checkbox(slotLabel.c_str(), &checked))
        {
            m_armorView.slotFiltererSelected.set(idx, checked);
            if (checked)
            {
                if (m_armorView.checkAllSlot)
                {
                    m_armorView.clearViewData();
                    m_armorView.checkAllSlot = false;
                }
                m_armorView.update_view_data(GetGenerator(), editing);
            }
            else if (!m_armorView.checkAllSlot)
            {
                if (m_armorView.slotFiltererSelected == 0)
                {
                    m_armorView.clearViewData();
                }
                else
                {
                    m_armorView.remove_armors_has_slot(
                        static_cast<Slot>(m_armorView.slotFiltererSelected.to_ulong()), static_cast<Slot>(1 << idx)
                    );
                }
            }
        }
    }
}

void OutfitEditPanel::OnAcceptAddArmorToOutfit(Context &context, const EditingOutfit &editingOutfit, const Armor *armor)
{
    if (editingOutfit.IsConflictWith(armor))
    {
        context.popupList.emplace_back(std::make_unique<OverrideArmorRequest>(editingOutfit.GetId(), armor));
    }
    else
    {
        +[&] {
            return m_outfitService.AddArmor(editingOutfit.GetId(), editingOutfit.GetName(), armor);
        };
        m_armorView.remove_armor(armor);
        m_armorView.multiSelection.Clear();
    }
}

void OutfitEditPanel::DeleteRequest::OnConfirm(OutfitEditPanel *editPanel)
{
    if (const auto opt = editPanel->m_uiData.GetOutfitList().GetOutfitById(id); opt.has_value())
    {
        +[&] {
            return editPanel->m_outfitService.DeleteArmor(opt.value().GetId(), opt.value().GetName(), armor);
        };
        assert(editPanel->m_armorView.add_armor(armor).has_value() && "Can't add armor!");
    }
    else
    {
        PushError(editPanel->m_uiData, Error::delete_armor_from_unknown_outfit_id);
    }
}

void OutfitEditPanel::DeleteRequest::DoDraw(SosUiData &uiData, bool &confirmed)
{
    if (!uiData.GetOutfitList().HasOutfit(id))
    {
        PushError(uiData, Error::delete_armor_from_unknown_outfit_id);
        return;
    }
    const auto message = Translation::Translate("$SkyOutSys_Confirm_RemoveArmor_Text{}", true, armor->GetName());
    ImGui::Text("%s", message.c_str());
    RenderConfirmButtons(confirmed);
}

void OutfitEditPanel::OverrideArmorRequest::OnConfirm(OutfitEditPanel *editPanel)
{
    if (const auto opt = editPanel->m_uiData.GetOutfitList().GetOutfitById(id); opt.has_value())
    {
        +[&] {
            return editPanel->m_outfitService.AddArmor(opt.value().GetId(), opt.value().GetName(), armor);
        };
        editPanel->m_armorView.remove_armor(armor);
        editPanel->m_armorView.multiSelection.Clear();
    }
    else
    {
        PushError(editPanel->m_uiData, Error::add_armor_to_unknown_outfit_id);
    }
}

void OutfitEditPanel::OverrideArmorRequest::DoDraw(SosUiData &uiData, bool &confirmed)
{
    if (!uiData.GetOutfitList().HasOutfit(id))
    {
        PushError(uiData, Error::add_armor_to_unknown_outfit_id);
        return;
    }
    const auto message = Translation::Translate("$SkyOutSys_Confirm_BodySlotConflict_Text");
    RenderMultilineMessage(message);
    RenderConfirmButtons(confirmed);
}

void OutfitEditPanel::BatchAddArmorsRequest::OnConfirm(OutfitEditPanel *editPanel)
{
    auto &outfitList = editPanel->m_uiData.GetOutfitList();
    if (!outfitList.HasOutfit(id))
    {
        PushError(editPanel->m_uiData, Error::add_armor_to_unknown_outfit_id);
        return;
    }
    SlotEnumeration usedSlot;
    // We will remove all used armors in view;
    auto &editingOutfit = outfitList.GetOutfitById(id).value();

    auto                                prevArmors = editingOutfit.GetUniqueArmors();
    std::vector<ArmorView::RankedArmor> newViewData;
    newViewData.reserve(editPanel->m_armorView.ViewData().size());
    for (size_t index = 0; index < editPanel->m_armorView.ViewData().size(); ++index)
    {
        const auto &armor = editPanel->m_armorView.ViewData().at(index);
        if (!editPanel->m_armorView.multiSelection.Contains(index) || usedSlot.all(armor->GetSlotMask()))
        {
            newViewData.emplace_back(armor);
            continue;
        }
        usedSlot.set(armor->GetSlotMask());
        if (editingOutfit.IsConflictWith(armor))
        {
            +[&] {
                return editPanel->m_outfitService.DeleteConflictArmors(editingOutfit.GetName(), armor);
            };
        }
        +[&] {
            return editPanel->m_outfitService.AddArmor(editingOutfit.GetId(), editingOutfit.GetName(), armor);
        };
    }
    editPanel->m_armorView.multiSelection.Clear();
    editPanel->m_armorView.SwapViewData(newViewData);

    const auto &armors = editingOutfit.GetUniqueArmors();
    std::erase_if(prevArmors, [&armors](const Armor *armor) {
        return armors.contains(armor);
    });
    for (const auto &armor : prevArmors)
    {
        if (auto result = editPanel->m_armorView.add_armor(armor); !result.has_value())
        {
            editPanel->m_uiData.PushErrorMessage(
                std::format("Can't restore armor {} from outfit {}", armor->GetName(), editingOutfit.GetName())
            );
        }
    }
}

void OutfitEditPanel::BatchAddArmorsRequest::DoDraw(SosUiData &, bool &confirmed)
{
    const auto message = Translation::Translate("$SosGui_BatchAddArmors_Message");
    RenderMultilineMessage(message);
    RenderConfirmButtons(confirmed);
}

bool OutfitEditPanel::SlotPolicyHelp::Draw(SosUiData &uiData, bool &confirmed, const ImGuiWindowFlags flags)
{
    const auto mainViewPort = ImGui::GetMainViewport();
    const auto maxSize      = ImVec2(mainViewPort->WorkSize.x * 0.5, mainViewPort->WorkSize.y * 0.5);
    ImGui::SetNextWindowSize(maxSize, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(maxSize.x * 0.5, maxSize.y * 0.5), ImGuiCond_Appearing);
    return ModalPopup::Draw(uiData, confirmed, flags);
}

void OutfitEditPanel::SlotPolicyHelp::DoDraw(SosUiData &, bool &)
{
    constexpr auto DrawEscaped = [](std::string &&str) {
        size_t pos = 0;
        while ((pos = str.find("\\n", pos)) != std::string::npos)
        {
            str.replace(pos, 2, "\n");
            pos += 1;
        }
        ImGui::TextWrapped("%s", str.c_str());
    };
    DrawEscaped(Translation::Translate("$SkyOutSys_OEdit_SlotPolicy_HelpText1"));
    DrawEscaped(Translation::Translate("$SkyOutSys_OEdit_SlotPolicy_HelpText2"));
    DrawEscaped(Translation::Translate("$SkyOutSys_OEdit_SlotPolicy_HelpText3"));
}

}