#include "gui/OutfitEditPanel.h"
#include "SosDataType.h"
#include "Translation.h"
#include "common/config.h"
#include "data/ArmorContainer.h"
#include "data/ArmorGenerator.h"
#include "data/SosUiData.h"
#include "data/SosUiOutfit.h"
#include "gui/FontSize.h"
#include "gui/Popup.h"
#include "gui/Table.h"
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
#include <cstdint>
#include <cstdlib>
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
    armorListShowAllSlotArmors  = false;
    armorGeneratorSelectedActor = nullptr;
    dirty                       = true;
}

void OutfitEditPanel::Render(const SosUiData::OutfitPair &wantEdit)
{
    if (m_editContext.dirty)
    {
        m_editContext.dirty = false;
        m_armorView.reset_view(GetGenerator());
    }
    if (ImGui::BeginChild("##OutfitEditPanelSideBar", {200, 0}, ImGuiUtil::ChildFlag().Borders().ResizeX().flags))
    {
        DrawSideBar(wantEdit.second);
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();
    {
        ImGui::PushID(this);
        RenderPopups(wantEdit);
        DrawOutfitArmors(wantEdit);
        DrawArmorGeneratorTabBar();
        DrawArmorViewFilter(wantEdit.second);
        DrawArmorView(wantEdit, m_armorView.viewData);
        ImGui::PopID();
    }
    ImGui::EndGroup();
}

void OutfitEditPanel::DrawSideBar(const SosUiOutfit *outfit)
{
    ImGuiUtil::TextScale("$SosGui_ModList", FONT_SIZE_TITLE_3);
    auto         itemHeight        = ImGui::GetTextLineHeight();
    static int   maxChildItemCount = 10;
    static float childHeight       = (itemHeight + ImGui::GetStyle().ItemInnerSpacing.y) * maxChildItemCount;

    if (ImGui::BeginChild("##ModNameListChild", {0, childHeight}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
    {
        std::function<void()> OnClick = [] {};
        for (const auto &[modName, count] : m_armorView.modRefCounter)
        {
            auto  label        = std::format("({}) {}", count, modName);
            bool  isSelected   = m_armorView.modNameFilterer == modName;
            float startCursorY = ImGui::GetCursorPosY();
            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                OnClick = [&, isSelected, modName] {
                    m_armorView.modNameFilterer = isSelected ? "" : modName;
                    m_armorView.reset_view(GetGenerator());
                };
            }
            itemHeight = std::max(itemHeight, ImGui::GetCursorPosY() - startCursorY);
        }
        childHeight = (itemHeight + ImGui::GetStyle().ItemInnerSpacing.y) * maxChildItemCount;
        OnClick();
    }
    ImGui::EndChild();

    ImGuiUtil::TextScale("$SosGui_BodySlots", FONT_SIZE_TITLE_3);
    if (ImGui::BeginChild("#SlotFilterChild", {0, childHeight}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY))
    {
        DrawArmorViewSlotFilterer(outfit);
    }
    ImGui::EndChild();

    if (m_armorView.multiSelection.Size == 1)
    {
        void   *it = nullptr;
        ImGuiID selectedRank; // must be name rank;
        m_armorView.multiSelection.GetNextSelectedItem(&it, &selectedRank);
        auto *armor = m_armorView.viewData.at(selectedRank);
        ImGuiUtil::TextScale("Armor Info", FONT_SIZE_TITLE_3);
        {
            ImGui::Indent();
            ImGui::BeginGroup();
            {
                ImGui::Text("%s", armor->GetName());
                ImGuiUtil::Value("FormID", armor->GetFormID());
                ImGui::Text("%s", util::GetFormModFileName(armor).data());
                ImGuiUtil::Value("$Armor Rating", armor->GetArmorRating());
                ImGuiUtil::Value("$WEIGHT", armor->GetWeight());
                ImGui::Text("%sPlayable", IsArmorNonPlayable(armor) ? "Non" : "");
                ImGui::BeginGroup();
                for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; ++slotPos)
                {
                    if (armor->HasPartOf(ToSlot(slotPos)))
                    {
                        ImGuiUtil::Button(Translation::Translate(get_slot_name_key(slotPos)).c_str());
                    }
                }
                ImGui::EndGroup();
            }
            ImGui::EndGroup();
        }
    }
}

void OutfitEditPanel::Refresh() {}

void OutfitEditPanel::Close()
{
    m_editContext.Clear();
    m_armorView.clear();
}

void OutfitEditPanel::OnSelectActor(const RE::Actor *, const SosUiOutfit *editingOutfit)
{
    if (editingOutfit == nullptr)
    {
        return;
    }
    m_armorView.reset_view(GetGenerator());
    m_armorView.remove_armors_in_outfit(editingOutfit);
}

void OutfitEditPanel::OnSelectOutfit(const SosUiOutfit *lastEditOutfit, const SosUiOutfit *editingOutfit)
{
    UpdateWindowTitle(editingOutfit->GetName());
    if (lastEditOutfit == nullptr)
    {
        m_armorView.init();
    }
    if (lastEditOutfit && dynamic_cast<BasicArmorGenerator *>(GetGenerator()) != nullptr)
    {
        if (lastEditOutfit->GetId() != editingOutfit->GetId())
        {
            m_armorView.add_armors_in_outfit(m_uiData, lastEditOutfit);
            m_armorView.remove_armors_in_outfit(editingOutfit);
            m_armorView.multiSelection.Clear();
        }
    }
    else
    {
        m_armorView.reset_view(GetGenerator());
        m_armorView.remove_armors_in_outfit(editingOutfit);
    }
}

void OutfitEditPanel::UpdateWindowTitle(const std::string &outfitName)
{
    m_windowTitle = Translation::Translate("$SosGui_WindowName_EditingOutfit");
    if (const auto pos = m_windowTitle.find("{}"); pos != std::string::npos)
    {
        m_windowTitle.replace(pos, 2, outfitName);
    }
}

void OutfitEditPanel::DrawOutfitArmors(const SosUiData::OutfitPair &wantEdit)
{
    if (wantEdit.second->IsEmpty())
    {
        ImGuiUtil::TextScale("$SosGui_EmptyHint{$ARMOR}", FONT_SIZE_TITLE_3);
        return;
    }

    ImGuiUtil::CheckBox("$SosGui_CheckBox_SlotFilter", &m_editContext.armorListShowAllSlotArmors);
    ImGui::SameLine();
    if (ImGuiUtil::Button("$SkyOutSys_OEdit_SlotPolicyHelp"))
    {
        m_slotPolicyHelp.Open();
    }

    static int selectedIdx = -1;
    if (constexpr auto flags = TableFlags().Resizable().SizingStretchProp().flags;
        ImGui::BeginTable("##OutfitArmors", 5, flags))
    {
        // clang-format off
        TableHeadersBuilder().Column("##Number").NoHide()
            .Column("$SosGui_TableHeader_Slot").NoSort()
            .Column("$ARMOR").NoSort()
            .Column("$SkyOutSys_OEdit_OutfitSettings_Header").WidthFixed().NoSort()
            .Column("$Delete").WidthFixed().NoSort()
            .CommitHeadersRow();
        // clang-format on

        for (uint32_t slotIdx = 0; slotIdx < SLOT_COUNT; ++slotIdx)
        {
            const auto &armor = wantEdit.second->GetArmorAt(slotIdx);
            if (!m_editContext.armorListShowAllSlotArmors && armor == nullptr)
            {
                continue;
            }

            ImGuiUtil::PushIdGuard idHolder(slotIdx);

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); // number column
            ImGui::Text("%d", slotIdx + 1);

            if (ImGui::TableNextColumn()) // slot name column
            {
                const bool isSelected = static_cast<uint32_t>(selectedIdx) == slotIdx;
                auto       name       = std::format("$SkyOutSys_BodySlot{}", slotIdx + SOS_SLOT_OFFSET);
                if (constexpr auto selectableFlags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
                    ImGuiUtil::Selectable(name, isSelected, selectableFlags))
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
            SlotPolicyCombo(wantEdit, slotIdx);

            ImGui::TableNextColumn(); // action column
            ImGui::BeginDisabled(armor == nullptr);
            {
                if (ImGuiUtil::Button("$Delete"))
                {
                    m_DeleteArmorPopup.wantDeleteArmor = armor;
                    m_DeleteArmorPopup.Open();
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndTable();
    }
}

void OutfitEditPanel::HighlightConflictSlot(const Slot slot) const
{
    if (m_armorView.multiSelectedSlot.any(slot))
    {
        auto *drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor(255, 0, 0, 255), 0,
                          ImDrawFlags_None, 2.0F);
    }
}

void OutfitEditPanel::SlotPolicyCombo(const SosUiData::OutfitPair &wantEdit, const uint32_t &slotIdx) const
{
    auto       &policyNameKey = wantEdit.second->GetSlotPolicies().at(slotIdx);
    std::string policyName;

    if (ImGui::BeginCombo("##SlotPolicy", Translation::Translate(policyNameKey).c_str(),
                          ImGuiComboFlags_WidthFitPreview))
    {
        for (const auto &policy :
             {SlotPolicy::Inherit, SlotPolicy::Passthrough, SlotPolicy::RequireEquipped, SlotPolicy::AlwaysUseOutfit})
        {
            policyName = SlotPolicyToUiString(policy);
            if (ImGui::Selectable(policyName.c_str(), false))
            {
                SlotPolicyToCode(policy);
                +[&] {
                    return m_outfitService.SetSlotPolicy(wantEdit.first, wantEdit.second->GetName(), slotIdx, policy);
                };
            }
            ImGuiUtil::SetItemTooltip(SlotPolicyToTooltipString(policy));
        }
        ImGui::EndCombo();
    }
}

inline auto OutfitEditPanel::get_slot_name_key(const uint32_t slotPos) -> std::string
{
    return std::format("$SkyOutSys_BodySlot{}", slotPos + SOS_SLOT_OFFSET);
}

bool OutfitEditPanel::ArmorFilter::PassFilter(const Armor *armor) const
{
    if (mustPlayable && IsArmorNonPlayable(armor))
    {
        return false;
    }
    if (!filter.IsActive())
    {
        return true;
    }
    if (DebounceInput::PassFilter(armor->GetName()))
    {
        return true;
    }
    // if (const auto *modFile = armor->GetFile(); modFile != nullptr)
    //{
    //     if (DebounceInput::PassFilter(modFile->GetFilename().data()))
    //     {
    //         return true;
    //     }
    // }
    return false;
}

bool OutfitEditPanel::ArmorFilter::Draw()
{
    bool needUpdate = ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &mustPlayable);
    ImGui::SameLine();
    needUpdate |= DebounceInput::Draw("##ArmorFilter",
                                      Translation::Translate("$SkyOutSys_OEdit_AddFromList_Filter_Name").c_str());
    return needUpdate;
}

void OutfitEditPanel::DrawArmorGeneratorTabBar()
{
    using namespace ImGuiUtil;
    if (ImGui::BeginTabBar("ArmorGeneratorTabBar"), TabBarFlags().DrawSelectedOverline().Reorderable().flags)
    {
        const auto *tabBar          = ImGui::GetCurrentTabBar();
        const auto  nextTabId       = tabBar->NextSelectedTabId;
        const auto  selectedId      = tabBar->SelectedTabId;
        auto        isTabItemAppear = [&] {
            const ImGuiTabItem *tabItem = ImGui::TabBarGetCurrentTab(ImGui::GetCurrentTabBar());
            return selectedId == 0 || (selectedId != nextTabId && tabItem->ID == nextTabId);
        };
        auto UpdateView = [&]() {
            m_armorView.modNameFilterer = "";
            m_armorView.reset_view(GetGenerator());
            m_armorView.reset_counter();
        };
        auto  *player        = RE::PlayerCharacter::GetSingleton();
        auto *&selectedActor = m_armorGeneratorTabBar.selectedActor;
        if (ImGui::BeginTabItem("From Actor Inventory"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = std::make_unique<InventoryArmorGenerator>(selectedActor);
                UpdateView();
            }
            Text("$SosGui_Actor_ArmorSource");
            ImGui::SameLine();
            if (widgets::DrawNearActorsCombo(m_uiData.GetNearActors(), &selectedActor, player))
            {
                m_armorGeneratorTabBar.generator = std::make_unique<InventoryArmorGenerator>(selectedActor);
                UpdateView();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("From Actor Carried Items"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                UpdateView();
            }
            Text("$SosGui_Actor_ArmorSource");
            ImGui::SameLine();
            if (widgets::DrawNearActorsCombo(m_uiData.GetNearActors(), &selectedActor, player))
            {
                m_armorGeneratorTabBar.generator = std::make_unique<CarriedArmorGenerator>(selectedActor);
                UpdateView();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("From form-id"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = nullptr;
                m_armorView.clearViewData();
            }
            ImGui::Text("0x");
            ImGui::SameLine();
            static std::array<char, 9> formIdBuf;
            if (ImGui::InputText("##FormIdInput", formIdBuf.data(), formIdBuf.size(),
                                 ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal))
            {
                m_armorView.clearViewData();
                char          *pEnd{};
                const uint32_t result = strtoul(formIdBuf.data(), &pEnd, 16);
                if (pEnd != formIdBuf.data())
                {
                    if (auto foundArmor = RE::TESForm::LookupByID<Armor>(result); foundArmor != nullptr)
                    {
                        m_armorView.viewData.emplace_back(foundArmor);
                    }
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("All Armors"))
        {
            if (isTabItemAppear())
            {
                m_armorGeneratorTabBar.generator = std::make_unique<BasicArmorGenerator>();
                UpdateView();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void OutfitEditPanel::DrawArmorViewTableContent(const std::vector<Armor *>                            &viewData,
                                                const std::function<void(Armor *armor, size_t index)> &drawAction)
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
                drawAction(viewData.at(index), index);
            }
        }
        else
        {
            using namespace std::views;
            size_t index = static_cast<size_t>(clipper.DisplayStart);
            for (int         count = clipper.DisplayEnd - clipper.DisplayStart;
                 const auto &armor : viewData | reverse | drop(clipper.DisplayStart) | take(count))
            {
                drawAction(armor, index++);
            }
        }
    }
    m_armorView.multiSelection.ApplyRequests(ImGui::EndMultiSelect());
}

void OutfitEditPanel::DrawArmorViewFilter(const SosUiOutfit *outfit)
{
    // filter armor name and mod name
    if (m_armorView.armorFilter.Draw())
    {
        m_armorView.armorFilter.dirty = false;
        m_armorView.reset_view(GetGenerator());
        m_armorView.remove_armors_in_outfit(outfit);
    }
}

void OutfitEditPanel::DrawArmorView(const SosUiData::OutfitPair &wantEdit, const std::vector<Armor *> &viewData)
{
    if (constexpr auto flags =
            TableFlags().ScrollY().Resizable().SizingFixedFit().Sortable().Hideable().Reorderable().flags;
        viewData.empty() || !ImGui::BeginTable("##ArmorCandidates", 6, flags))
    {
        return;
    }

    // clang-format off
    TableHeadersBuilder().Column("##Number").NoSort()
        .Column("$ARMOR").DefaultSort().NoHide()
        .Column("FormID").NoSort()
        .Column("ModName").NoSort()
        .Column("Playable").NoSort()
        .Column("$Add").WidthFixed().NoSort()
        .CommitHeadersRow();
    // clang-format on

    std::function<void()> onRequireAddArmor = [] {};

    auto drawAction = [&](Armor *armor, const size_t index) {
        ImGuiUtil::PushIdGuard idGuard(index);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); // number column
        {
            const bool isSelected = m_armorView.multiSelection.Contains(static_cast<ImGuiID>(index));
            ImGui::SetNextItemSelectionUserData(index);
            constexpr auto flags = ImGuiUtil::SelectableFlag().AllowOverlap().SpanAllColumns().flags;
            ImGui::Selectable(std::format("{}", index + 1).c_str(), isSelected, flags);
            if (isSelected)
            {
                m_armorView.multiSelectedSlot.set(armor->GetSlotMask());
            }
            else
            {
                m_armorView.multiSelectedSlot.reset(armor->GetSlotMask());
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGuiUtil::MenuItem("$SosGui_ContextMenu_AddAllArmor"))
                {
                    if (m_armorView.multiSelection.Size == 1)
                    {
                        onRequireAddArmor = [&, armor] {
                            OnAcceptAddArmorToOutfit(wantEdit, armor);
                        };
                    }
                    else if (m_armorView.multiSelection.Size > 1)
                    {
                        onRequireAddArmor = [&] {
                            m_batchAddArmorsPopUp.Open();
                        };
                    }
                }
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
            ImGui::Text("%s", IsArmorNonPlayable(armor) ? "\xe2\x9d\x8c" : "\xe2\x9c\x85");
        }

        if (ImGui::TableNextColumn() && ImGuiUtil::Button("$Add")) // column Action
        {
            onRequireAddArmor = [&, armor] {
                OnAcceptAddArmorToOutfit(wantEdit, armor);
            };
        }
    };

    DrawArmorViewTableContent(viewData, drawAction);
    ImGui::EndTable();
    onRequireAddArmor();
}

void OutfitEditPanel::DrawArmorViewModNameFilterer()
{
    const auto hint    = Translation::Translate("$SosGui_Combo_ModNameFilter");
    const auto preview = m_armorView.modNameFilterer;
    if (!ImGui::BeginCombo("##ModNameFilter", preview.empty() ? hint.c_str() : preview.data(),
                           ImGuiComboFlags_WidthFitPreview))
    {
        return;
    }
    if (ImGuiUtil::Selectable("$Cancel", false))
    {
        m_armorView.modNameFilterer = "";
        if (!m_armorView.modNameFilterer.empty())
        {
            m_armorView.reset_view(GetGenerator());
        }
    }
    std::function<void()> OnClick = [] {};
    for (const auto &[modName, refCount] : m_armorView.modRefCounter)
    {
        if (refCount == 0) continue;
        auto label = std::format("{} ({})", modName, m_armorView.modRefCounter[modName]);
        if (ImGui::Selectable(label.c_str(), false))
        {
            OnClick = [&, modName] {
                m_armorView.modNameFilterer = modName;
                m_armorView.reset_view(GetGenerator());
            };
        }
    }
    OnClick();
    ImGui::EndCombo();
}

void OutfitEditPanel::DrawArmorViewSlotFilterer(const SosUiOutfit *outfit)
{
    std::string name = std::format("All({}/{})##AllSlot", m_armorView.viewData.size(), m_armorView.availableArmorCount);
    if (ImGui::Checkbox(name.c_str(), &m_armorView.checkAllSlot))
    {
        if (!m_armorView.checkAllSlot)
        {
            if (m_armorView.selectedFilterSlot == 0)
            {
                m_armorView.clearViewData();
            }
            else
            {
                auto slotValue = m_armorView.selectedFilterSlot.to_ulong();
                m_armorView.remove_armors_has_slot(static_cast<Slot>(slotValue), static_cast<Slot>(~slotValue));
            }
        }
        else
        {
            m_armorView.reset_view(GetGenerator());
            m_armorView.remove_armors_in_outfit(outfit);
        }
    }

    auto slotLabel = [this](uint8_t slotPos) {
        return std::format("({}) {}", m_armorView.slotCounter[slotPos],
                           Translation::Translate(get_slot_name_key(slotPos)));
    };
    for (uint8_t idx = 0; idx < RE::BIPED_OBJECT::kEditorTotal; ++idx)
    {
        ImGuiUtil::PushIdGuard idGuard(idx);

        bool checked = m_armorView.selectedFilterSlot.test(idx);
        if (ImGui::Checkbox(slotLabel(idx).c_str(), &checked))
        {
            m_armorView.selectedFilterSlot.set(idx, checked);
            if (checked)
            {
                m_armorView.multiSelection.Clear();
                if (m_armorView.checkAllSlot)
                {
                    m_armorView.clearViewData();
                    m_armorView.checkAllSlot = false;
                }
                m_armorView.reset_view(GetGenerator());
            }
            else if (!m_armorView.checkAllSlot)
            {
                m_armorView.multiSelection.Clear();
                if (m_armorView.selectedFilterSlot == 0)
                {
                    m_armorView.clearViewData();
                }
                else
                {
                    m_armorView.remove_armors_has_slot(static_cast<Slot>(m_armorView.selectedFilterSlot.to_ulong()),
                                                       static_cast<Slot>(1 << idx));
                }
            }
        }
    }
}

void OutfitEditPanel::BatchAddArmors(const SosUiData::OutfitPair &wantEdit)
{
    m_armorView.add_armors_in_outfit(m_uiData, wantEdit.second); // first, restore all removed armors in outfit
    SlotEnumeration usedSlot;
    // We will remove all used armors in view;
    std::vector<Armor *> newViewData;
    newViewData.reserve(m_armorView.viewData.size());
    for (size_t index = 0; index < m_armorView.viewData.size(); ++index)
    {
        auto *armor = m_armorView.viewData.at(index);
        if (!m_armorView.multiSelection.Contains(index) || usedSlot.all(armor->GetSlotMask()))
        {
            newViewData.emplace_back(armor);
            continue;
        }
        usedSlot.set(armor->GetSlotMask());
        if (wantEdit.second->IsConflictWith(armor))
        {
            +[&] {
                return m_outfitService.DeleteConflictArmors(wantEdit.second->GetName(), armor);
            };
        }
        +[&] {
            return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
        };
    }
    m_armorView.multiSelection.Clear();
    m_armorView.viewData.swap(newViewData);
}

auto OutfitEditPanel::RenderOutfitAddPolicyById(const SosUiData::OutfitPair &wantEdit,
                                                const bool                  &fFilterPlayable) const -> void
{
    static std::array<char, 32> formIdBuf;
    static char                *pEnd{};

    ImGui::PushID("AddByFormId");
    ImGui::Text("0x");
    ImGui::SameLine();
    ImGui::InputText("##InputArmorId", formIdBuf.data(), formIdBuf.size(),
                     ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal);
    const auto formId = std::strtoul(formIdBuf.data(), &pEnd, 16);
    if (Armor *armor = nullptr; *pEnd == 0 && (armor = RE::TESForm::LookupByID<Armor>(formId)) != nullptr)
    {
        ImGui::Text("%s", armor->GetName());
        ImGui::SameLine();
        ImGui::BeginDisabled(fFilterPlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0);
        if (ImGuiUtil::Button("$Add"))
        {
            +[&] {
                return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
            };
        }
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    ImGui::PopID();
}

void OutfitEditPanel::GetArmorGeneratorFromPolicy(const OutfitAddPolicy policy, ArmorGenerator **generator)
{
    switch (policy)
    {
        case OutfitAddPolicy_AddFromCarried:
            *generator = new CarriedArmorGenerator(RE::PlayerCharacter::GetSingleton());
            break;

        case OutfitAddPolicy_AddFromInventory:
            *generator = new InventoryArmorGenerator(RE::PlayerCharacter::GetSingleton());
            break;

        case OutfitAddPolicy_AddByID:
            break;

        case OutfitAddPolicy_AddAny:
            *generator = new BasicArmorGenerator();
            break;

        default:
            break;
    }
}

void OutfitEditPanel::ArmorView::init()
{
    auto       *dataHandler = RE::TESDataHandler::GetSingleton();
    const auto &armorArray  = dataHandler->GetFormArray<RE::TESObjectARMO>();

    availableArmorCount = 0;
    for (const auto &armor : armorArray)
    {
        if (IsArmorCanDisplay(armor))
        {
            armorContainer.Insert(armor);
            availableArmorCount++;
        }
    }
}

void OutfitEditPanel::ArmorView::clear()
{
    armorContainer.Clear();
    viewData.clear();
    modRefCounter.clear();
    slotCounter.fill(0);
    selectedFilterSlot = 0;
    checkAllSlot       = true;
    multiSelection.Clear();
    multiSelectedSlot = Slot::kNone;
    armorFilter.clear();
    modNameFilterer = "";
    modRefCounter.clear();
}

void OutfitEditPanel::ArmorView::clearViewData()
{
    viewData.clear();
    multiSelection.Clear();
}

void OutfitEditPanel::ArmorView::remove_armors_has_slot(const Slot selectedSlots, const Slot toRemoveSlot)
{
    multiSelection.Clear();
    for (auto itBegin = viewData.begin(); itBegin != viewData.end();)
    {
        if (const auto *armor = *itBegin;
            armor->HasPartOf(toRemoveSlot) && util::IsArmorHasNoneSlotOf(armor, selectedSlots))
        {
            itBegin = viewData.erase(itBegin);
        }
        else
        {
            ++itBegin;
        }
    }
}

void OutfitEditPanel::ArmorView::add_armors_in_outfit(SosUiData &uiData, const SosUiOutfit *editingOutfit)
{
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (auto *armor = editingOutfit->GetArmorAt(slotPos); armor != nullptr)
        {
            if (auto result = add_armor(armor); !result.has_value())
            {
                // this error is because some armor has multiple slots, just ignore it;
                if (result.error() == error::armor_already_exists)
                {
                    continue;
                }
                uiData.PushErrorMessage(std::format("unexpected error when add_armors_in_outfit: {}", //
                                                    ToErrorMessage(result.error())));
            }
        }
    }
}

void OutfitEditPanel::ArmorView::remove_armors_in_outfit(const SosUiOutfit *editingOutfit)
{
    // may multi armor use the same slot
    for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
    {
        if (const auto *armor = editingOutfit->GetArmorAt(slotPos); armor != nullptr)
        {
            if (auto result = find_armor(armor); result.has_value())
            {
                viewData.erase(viewData.begin() + result.value());
                slotCounter[slotPos] -= 1;
            }
        }
    }
}

// true pass, false discard
bool OutfitEditPanel::ArmorView::filter(const Armor *armor) const
{
    if (!modNameFilterer.empty())
    {
        if (util::GetFormModFileName(armor) != modNameFilterer)
        {
            return false;
        }
    }
    // filter by slot: has any selected slot
    auto slot = static_cast<Slot>(selectedFilterSlot.to_ulong());
    if (!checkAllSlot && slot != Slot::kNone && util::IsArmorHasNoneSlotOf(armor, slot))
    {
        return false;
    }
    // filter by armor name
    if (!armorFilter.PassFilter(armor))
    {
        return false;
    }
    return true;
}

auto OutfitEditPanel::IsArmorCanDisplay(const Armor *armor) -> bool
{
    bool canDisplay = false;
    if (armor != nullptr && armor->templateArmor == nullptr)
    {
        if (const std::string_view name = armor->GetName(); !name.empty())
        {
            canDisplay = true;
        }
    }

    return canDisplay;
}

auto OutfitEditPanel::ArmorView::add_armor(Armor *armor) -> std::expected<void, error>
{
    if (filter(armor))
    {
        const auto armorRank = armorContainer.GetRank(armor->formID);
        size_t     startPos  = 0;
        size_t     endPos    = viewData.size();
        while (endPos - startPos > 0)
        {
            const size_t middle = startPos + (endPos - startPos) / 2;
            const auto   rank   = armorContainer.GetRank(viewData.at(middle)->formID);
            if (rank >= armorContainer.Size())
            {
                return std::unexpected{error::unassociated_armor};
            }
            if (armorRank < rank)
            {
                endPos = middle;
            }
            else if (armorRank > rank)
            {
                startPos = middle + 1;
            }
            else
            {
                return std::unexpected{error::armor_already_exists};
            }
        }
        viewData.insert(viewData.begin() + startPos, armor);
    }
    return {};
}

bool OutfitEditPanel::ArmorView::remove_armor(const Armor *armor)
{
    if (const auto result = find_armor(armor); result.has_value())
    {
        viewData.erase(viewData.begin() + result.value());
        for (uint8_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            if (armor->HasPartOf(ToSlot(slotPos)))
            {
                slotCounter[slotPos] -= 1;
            }
        }
        return true;
    }
    return false;
}

void OutfitEditPanel::ArmorView::reset_counter()
{
    modRefCounter.clear();
    for (const auto &armor : viewData)
    {
        std::string_view modName = util::GetFormModFileName(armor);
        modRefCounter.emplace(modName, 0);
        modRefCounter[modName] += 1;

        auto slotMaskValue = static_cast<uint32_t>(armor->GetSlotMask());
        for (uint32_t slotPos = 0; slotPos < RE::BIPED_OBJECT::kEditorTotal; slotPos++)
        {
            uint32_t slotValue = 1 << slotPos;
            if (slotValue > slotMaskValue) break;
            if ((slotMaskValue & slotValue) != 0)
            {
                slotCounter[slotPos]++;
            }
        }
    }
}

void OutfitEditPanel::ArmorView::reset_view(ArmorGenerator *generator)
{
    if (generator != nullptr)
    {
        clearViewData();
        generator->for_each([&](Armor *armor) {
            if (const auto result = add_armor(armor); !result.has_value())
            {
                throw std::runtime_error(ToErrorMessage(result.error()));
            }
        });
    }
}

auto OutfitEditPanel::ArmorView::find_armor(const Armor *armor) const -> std::expected<size_t, error>
{
    const auto armorRank = armorContainer.GetRank(armor->formID);
    size_t     startPos  = 0;
    size_t     endPos    = viewData.size();
    while (endPos - startPos > 0)
    {
        const size_t middle = (endPos + startPos) / 2;
        const auto   rank   = armorContainer.GetRank(viewData.at(middle)->formID);
        if (rank >= armorContainer.Size())
        {
            return std::unexpected{error::unassociated_armor};
        }
        if (armorRank < rank)
        {
            endPos = middle;
        }
        else if (armorRank > rank)
        {
            startPos = middle + 1;
        }
        else
        {
            return middle;
        }
    }
    return std::unexpected{error::armor_not_exist};
}

bool OutfitEditPanel::ArmorView::no_select_any_slot() const
{
    return !checkAllSlot && selectedFilterSlot == 0;
}

void OutfitEditPanel::OnAcceptAddArmorToOutfit(const SosUiData::OutfitPair &wantEdit, Armor *armor)
{
    if (wantEdit.second->IsConflictWith(armor))
    {
        m_ConflictArmorPopup.conflictedArmor = armor;
        m_ConflictArmorPopup.Open();
    }
    else
    {
        +[&] {
            return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(), armor);
        };
        m_armorView.remove_armor(armor);
        m_armorView.multiSelection.Clear();
    }
}

void OutfitEditPanel::RenderPopups(const SosUiData::OutfitPair &wantEdit)
{
    ImGui::PushStyleVarX(ImGuiStyleVar_WindowPadding, 25.0F);
    ImGui::PushFontSize(FONT_SIZE_TITLE_4);
    bool confirmed = false;
    if (m_ConflictArmorPopup.Draw(confirmed); confirmed)
    {
        +[&] {
            return m_outfitService.AddArmor(wantEdit.first, wantEdit.second->GetName(),
                                            m_ConflictArmorPopup.conflictedArmor);
        };
        m_armorView.remove_armor(m_ConflictArmorPopup.conflictedArmor);
        m_ConflictArmorPopup.conflictedArmor = nullptr;
    }

    if (m_DeleteArmorPopup.Draw(confirmed); confirmed)
    {
        +[&] {
            return m_outfitService.DeleteArmor(wantEdit.first, wantEdit.second->GetName(),
                                               m_DeleteArmorPopup.wantDeleteArmor);
        };
        assert(m_armorView.add_armor(m_DeleteArmorPopup.wantDeleteArmor).has_value() && "Can't add armor!");
        m_DeleteArmorPopup.wantDeleteArmor = nullptr;
    }
    m_slotPolicyHelp.Draw();
    confirmed = false;
    if (m_batchAddArmorsPopUp.Draw(confirmed); confirmed)
    {
        BatchAddArmors(wantEdit);
    }

    ImGui::PopFontSize();
    ImGui::PopStyleVar();
}
}