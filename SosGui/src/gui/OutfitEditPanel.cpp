#include "gui/OutfitEditPanel.h"

#include "SosDataType.h"
#include "Translation.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/UiSettings.h"
#include "gui/icon.h"
#include "gui/popup/Popup.h"
#include "gui/widgets.h"
#include "i18n/translator_manager.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/imguiex_enum_wrap.h"
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

namespace SosGui
{
namespace
{
constexpr int SOS_SLOT_OFFSET = 30;

inline auto get_slot_name_key(const uint32_t slotPos) -> std::string
{
    return std::format("Panels.OutfitEdit.Slot{}", slotPos + SOS_SLOT_OFFSET);
}
} // namespace

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

void OutfitEditPanel::Focus()
{
    ImGui::SetWindowFocus(m_windowTitle.c_str());
    BaseGui::Focus();
}

void OutfitEditPanel::OnSelectOutfit(const EditingOutfit &lastEdit, const EditingOutfit &editing)
{
    UpdateWindowTitle(editing.GetName());
    if (dynamic_cast<BasicArmorGenerator *>(GetGenerator()) != nullptr)
    {
        if (lastEdit.GetId() != editing.GetId())
        {
            m_armorView.add_armors_in_outfit(lastEdit.GetSourceOutfit());
            m_armorView.remove_armors_in_outfit(editing.GetSourceOutfit());
            m_armorView.multiSelection.Clear();
        }
    }
    else
    {
        m_armorView.update_view_data(GetGenerator(), editing.GetSourceOutfit());
    }
}

void OutfitEditPanel::PushError(const Error error)
{
    switch (error)
    {
        case Error::delete_armor_from_unknown_outfit_id:
            ErrorNotifier::GetInstance().Error("Can't delete a invalid or has been removed outfit.");
            break;
        case Error::add_armor_to_unknown_outfit_id:
            ErrorNotifier::GetInstance().Error("Can't add armor to a invalid or has been removed outfit.");
            break;
    }
}

void OutfitEditPanel::Draw(Context &context, const EditingOutfit &editingOutfit)
{
    if (!IsShowing())
    {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(DEFAULT_OUTFIT_EDIT_WINDOW_POS_X, DEFAULT_WINDOW_POS_Y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin(m_windowTitle.c_str(), &m_show))
    {
        if (m_editContext.dirty)
        {
            m_editContext.dirty = false;
            m_armorView.reset_view(GetGenerator(), editingOutfit.GetSourceOutfit());
        }
        if (ImGui::BeginChild("##OutfitEditPanelSideBar", {200, 0}, ImGuiEx::ChildFlags().Borders().ResizeX()))
        {
            DrawSideBar(editingOutfit.GetSourceOutfit());
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginGroup();
        DrawOutfitPanel(context, editingOutfit);
        DrawArmorGeneratorTabBar(editingOutfit.GetSourceOutfit());
        DrawArmorViewFilter(editingOutfit.GetSourceOutfit());
        DrawArmorView(context, editingOutfit);
        ImGui::EndGroup();
    }
    ImGui::End();
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
    ImGui::TextUnformatted(editingOutfit.GetName().c_str());
    if (ImGui::BeginTabBar("##OutfitTabBarView"))
    {
        if (ImGui::BeginTabItem(Translate1("Armor"), nullptr, ImGuiTabItemFlags_Leading))
        {
            DrawOutfitArmors(context, editingOutfit);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::DrawSideBar(const SosUiOutfit *editingOutfit)
{
    ImGui::TextUnformatted(Translate1("Panels.Outfit.ModList"));
    static int maxChildItemCount = 10;
    const auto itemHeight        = ImGui::GetTextLineHeight();
    float      childHeight       = (itemHeight + ImGui::GetStyle().ItemInnerSpacing.y) * maxChildItemCount;

    if (ImGui::BeginChild("##ModNameListChild", {0, childHeight}, ImGuiEx::ChildFlags().Borders().ResizeY()))
    {
        DrawArmorViewModNameFilterer(editingOutfit);
    }
    ImGui::EndChild();

    ImGuiUtil::Text(Translate1("Panels.Outfit.BodySlots"));
    if (ImGui::BeginChild("#SlotFilterChild", {0, childHeight}, ImGuiEx::ChildFlags().Borders().ResizeY()))
    {
        DrawArmorViewSlotFilterer(editingOutfit);
    }
    ImGui::EndChild();
}

void OutfitEditPanel::UpdateWindowTitle(const std::string &outfitName)
{
    m_windowTitle = std::format("{} - {}###OutfitEditPanel", Translate("Panels.OutfitEdit.EditingHint"), Translate("Panels.OutfitEdit.Title"));
}

void OutfitEditPanel::DrawOutfitArmors(Context &context, const EditingOutfit &editingOutfit)
{
    if (editingOutfit.IsUntitled())
    {
        ImGuiUtil::Text(Translate("Panels.OutfitEdit.MissingOutfitHint"));
        return;
    }
    if (editingOutfit.IsEmpty())
    {
        ImGuiUtil::Text(Translate("Panels.OutfitEdit.EmptyHint"));
        return;
    }

    ImGui::Checkbox(Translate1("Panels.OutfitEdit.ShowAllSlots"), &m_editContext.ShowAllSlotOutfitArmors);
    ImGui::SameLine();
    if (ImGui::Button(Translate1("Panels.OutfitEdit.SlotPolicies")))
    {
        context.popupList.push_back(std::make_unique<SlotPolicyHelp>());
    }

    static uint32_t selectedIdx = RE::BIPED_OBJECT::kEditorTotal;
    if (ImGui::BeginTable("##OutfitArmors", 5, ImGuiEx::TableFlags().Resizable().SizingStretchProp()))
    {
        ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoHide());
        ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.Slot"), ImGuiEx::TableColumnFlags().NoSort());
        ImGui::TableSetupColumn(Translate1("Armor"), ImGuiEx::TableColumnFlags().NoSort());
        ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.SlotPolicies"), ImGuiEx::TableColumnFlags().WidthFixed().NoSort());
        ImGui::TableSetupColumn(Translate1("Delete"), ImGuiEx::TableColumnFlags().WidthFixed().NoSort());
        ImGui::TableHeadersRow();

        for (uint32_t slotIdx = 0; slotIdx < RE::BIPED_OBJECT::kEditorTotal; ++slotIdx)
        {
            const auto *armor = editingOutfit.GetArmorAt(slotIdx);
            if (!m_editContext.ShowAllSlotOutfitArmors && armor == nullptr)
            {
                continue;
            }

            ImGui::PushID(static_cast<int>(slotIdx));

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%d", slotIdx + 1);

            if (ImGui::TableNextColumn()) // slot name column
            {
                const bool isSelected = selectedIdx == slotIdx;
                if (ImGui::Selectable(Translate1(get_slot_name_key(slotIdx)), isSelected, ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns()))
                {
                    selectedIdx = isSelected ? RE::BIPED_OBJECT::kEditorTotal : slotIdx;
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
                ImGui::BeginDisabled(armor == nullptr);
                if (ImGui::Button(Translate1("Delete")))
                {
                    context.popupList.emplace_back(std::make_unique<DeleteRequest>(editingOutfit.GetId(), armor));
                }
                ImGui::EndDisabled();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void OutfitEditPanel::HighlightConflictSlot(const Slot slot) const
{
    if (m_armorView.multiSelection.IsSelectSlot(slot))
    {
        auto *drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 0, 0, 255), 0, ImDrawFlags_None, 2.0F);
    }
}

void OutfitEditPanel::SlotPolicyCombo(const EditingOutfit &editingOutfit, const uint32_t &slotIdx) const
{
    auto       &policyNameKey = editingOutfit.GetSlotPolicies().at(slotIdx);
    std::string policyName;

    if (ImGui::BeginCombo("##SlotPolicy", Translation::Translate(policyNameKey).c_str(), ImGuiComboFlags_WidthFitPreview))
    {
        for (const auto &policy : {SlotPolicy::Inherit, SlotPolicy::Passthrough, SlotPolicy::RequireEquipped, SlotPolicy::AlwaysUseOutfit})
        {
            policyName = SlotPolicyToUiString(policy);
            if (ImGui::Selectable(policyName.c_str(), false))
            {
                if (!editingOutfit.IsUntitled())
                {
                    +[&] {
                        return m_outfitService.SetSlotPolicy(editingOutfit.GetId(), editingOutfit.GetName(), slotIdx, policy);
                    };
                }
            }
            ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
        }
        ImGui::EndCombo();
    }
}

void OutfitEditPanel::DrawArmorGeneratorTabBar(const SosUiOutfit *editingOutfit)
{
    ImGui::Separator();
    ImGuiUtil::Text(Translate("Panels.OutfitEdit.ArmomrGenerator.Title"));
    if (ImGui::BeginTabBar("ArmorGeneratorTabBar", ImGuiEx::TabBarFlags().DrawSelectedOverline().Reorderable()))
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
        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmomrGenerator.NearObjectInventory")))
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

            if (ImGui::BeginCombo(
                    Translate1("Panels.OutfitEdit.ArmomrGenerator.NearObjects"), wantVisit->GetDisplayFullName(), ImGuiComboFlags_WidthFitPreview
                ))
            {
                for (size_t index = 0; index < nearObjects.size(); ++index)
                {
                    ImGui::PushID(static_cast<int>(index));
                    if (ImGui::Selectable(nearObjects[index]->GetName(), generator->WantVisitIndex() == index))
                    {
                        generator->SetWantVisitIndex(index);
                        m_armorView.reset_view(GetGenerator(), editingOutfit);
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmomrGenerator.NearNpcCarried")))
        {
            if (isTabItemAppear())
            {
                armorGenerator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            if (widgets::DrawNearActorsCombo(m_uiData.GetNearActors(), &selectedActor, player))
            {
                armorGenerator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmomrGenerator.ByFormID")))
        {
            if (isTabItemAppear())
            {
                armorGenerator = nullptr;
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::Text("0x");
            ImGui::SameLine();
            static std::array<char, 9> formIdBuf;

            if (ImGui::InputText("##FormIdInput", formIdBuf.data(), formIdBuf.size(), ImGuiEx::InputTextFlags().CharsUppercase().CharsHexadecimal()))
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
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(Translate1("Panels.OutfitEdit.ArmomrGenerator.AllArmors")))
        {
            if (isTabItemAppear())
            {
                armorGenerator = std::make_unique<BasicArmorGenerator>();
                m_armorView.reset_view(GetGenerator(), editingOutfit);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::DrawArmorViewFilter(const SosUiOutfit *editingOutfit)
{
    {
        const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({5.0f, 5.0f}).Color<ImGuiCol_Button>(ImVec4{});
        if (ImGuiUtil::IconButton(ICON_FILE_PLUS_CORNER))
        {
            ImGui::OpenPopup("Filters");
        }
        ImGui::SetItemTooltip("%s", Translate1("Panels.OutfitEdit.Filterers"));
    }
    ImGui::SetNextWindowPos({ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y});
    if (ImGui::BeginPopup("Filters"))
    {
        bool needUpdate = ImGui::Checkbox(Translate1("Panels.OutfitEdit.Playable"), &m_armorView.armorFilter.mustPlayable);
        needUpdate |= ImGui::Checkbox(Translate1("Panels.OutfitEdit.Template"), &Settings::UiSettings::GetInstance()->includeTemplateArmor);
        if (needUpdate)
        {
            m_armorView.update_view_data(GetGenerator(), editingOutfit);
        }
        ImGui::EndPopup();
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
    constexpr auto flags =
        ImGuiEx::TableFlags().RowBg().BordersInnerH().ScrollY().Resizable().SizingStretchProp().Sortable().Hideable().Reorderable();
    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_FramePadding>({3.0F, 3.0F});
    if (ImGui::BeginTable("##ArmorCandidates", 6, flags))
    {
        DrawArmorViewContent(context, editingOutfit, m_armorView.ViewData());
        ImGui::EndTable();
    }
}

void OutfitEditPanel::DrawArmorViewContent(Context &context, const EditingOutfit &editingOutfit, const std::vector<ArmorView::RankedArmor> &viewData)
{
    using namespace ImGuiUtil;
    ImGui::TableSetupScrollFreeze(1, 1);
    ImGui::TableSetupColumn("##Number", ImGuiEx::TableColumnFlags().NoSort().WidthFixed());
    ImGui::TableSetupColumn(Translate1("Armor"), ImGuiEx::TableColumnFlags().DefaultSort().NoHide());
    ImGui::TableSetupColumn("FormID", ImGuiEx::TableColumnFlags().NoSort());
    ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.ModName"), ImGuiEx::TableColumnFlags().NoSort());
    ImGui::TableSetupColumn(Translate1("Panels.OutfitEdit.Playable"), ImGuiEx::TableColumnFlags().NoSort());
    ImGui::TableSetupColumn(Translate1("Add"), ImGuiEx::TableColumnFlags().NoSort());
    ImGui::TableHeadersRow();

    std::function<void()> onRequireAddArmor = [] {};

    auto drawArmorEntry = [&](const Armor *armor, const size_t index) {
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); // number column
        {
            const bool isSelected = m_armorView.multiSelection.UpdateSelected(armor, index);
            ImGui::SetNextItemSelectionUserData(static_cast<int64_t>(index));
            ImGui::Selectable(std::format("{}", index + 1).c_str(), isSelected, ImGuiEx::SelectableFlags().AllowOverlap().SpanAllColumns());

            if (ImGui::BeginPopupContextItem("Context"))
            {
                ImGui::BeginDisabled(editingOutfit.IsUntitled());
                if (ImGui::MenuItem(Translate1("Paneles.OutfitEdit.AddAll")))
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
                ImGui::EndDisabled();
                ImGui::EndPopup();
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
            if (IsArmorNonPlayable(armor))
            {
                ImGuiUtil::IconButton(ICON_X);
            }
            else
            {
                ImGuiUtil::IconButton(ICON_CHECK);
            }
        }

        ImGui::BeginDisabled(editingOutfit.IsUntitled());
        if (ImGui::TableNextColumn() && ImGui::Button(Translate1("Add"))) // column Action
        {
            onRequireAddArmor = [&, armor] {
                OnAcceptAddArmorToOutfit(context, editingOutfit, armor);
            };
        }
        ImGui::EndDisabled();
        ImGui::PopID();
    };

    DrawArmorViewTableContent(viewData, drawArmorEntry);
    if (!editingOutfit.IsUntitled())
    {
        onRequireAddArmor();
    }
}

void OutfitEditPanel::DrawArmorViewTableContent(const std::vector<ArmorView::RankedArmor> &viewData, const DrawArmorEntry &drawArmorEntry)
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
    const std::string name = std::format("All({}/{})##AllSlot", m_armorView.ViewData().size(), m_armorView.availableArmorCount);
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
        ImGui::PushID(idx);

        auto slotLabel = std::format("({}) {}", m_armorView.slotCounter[idx], Translation::Translate(get_slot_name_key(idx)));
        bool checked   = m_armorView.slotFiltererSelected.test(idx);
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
                    m_armorView.remove_armors_has_slot(static_cast<Slot>(m_armorView.slotFiltererSelected.to_ulong()), static_cast<Slot>(1 << idx));
                }
            }
        }
        ImGui::PopID();
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
        PushError(Error::delete_armor_from_unknown_outfit_id);
    }
}

void OutfitEditPanel::DeleteRequest::DoDraw(SosUiData &uiData, bool &confirmed)
{
    if (!uiData.GetOutfitList().HasOutfit(id))
    {
        PushError(Error::delete_armor_from_unknown_outfit_id);
        return;
    }
    ImGuiUtil::Text(std::format("{} {}?", Translate("Panels.OutfitEdit.RemoveArmor"), armor->GetName()));
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
        PushError(Error::add_armor_to_unknown_outfit_id);
    }
}

void OutfitEditPanel::OverrideArmorRequest::DoDraw(SosUiData &uiData, bool &confirmed)
{
    if (!uiData.GetOutfitList().HasOutfit(id))
    {
        PushError(Error::add_armor_to_unknown_outfit_id);
        return;
    }
    RenderMultilineMessage(Translate("Paneles.OutfitEdit.ConfirmRemoveConfictArmors"));
    RenderConfirmButtons(confirmed);
}

void OutfitEditPanel::BatchAddArmorsRequest::OnConfirm(OutfitEditPanel *editPanel)
{
    auto &outfitList = editPanel->m_uiData.GetOutfitList();
    if (!outfitList.HasOutfit(id))
    {
        PushError(Error::add_armor_to_unknown_outfit_id);
        return;
    }
    SlotEnumeration usedSlot;
    // We will remove all used armors in view;
    auto           &editingOutfit = outfitList.GetOutfitById(id).value();

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
            ErrorNotifier::GetInstance().Error(std::format("Can't restore armor {} from outfit {}", armor->GetName(), editingOutfit.GetName()));
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
    const auto maxSize      = ImVec2(mainViewPort->WorkSize.x * 0.5F, mainViewPort->WorkSize.y * 0.5F);
    ImGui::SetNextWindowSize(maxSize, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(maxSize.x * 0.5F, maxSize.y * 0.5F), ImGuiCond_Appearing);
    return ModalPopup::Draw(uiData, confirmed, flags);
}

void OutfitEditPanel::SlotPolicyHelp::DoDraw(SosUiData &, bool &)
{
    ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicyHelpText1"));
    ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicyHelpText2"));
    ImGui::TextWrapped("%s", Translate1("Panels.OutfitEdit.SlotPolicyHelpText3"));
}
} // namespace SosGui
