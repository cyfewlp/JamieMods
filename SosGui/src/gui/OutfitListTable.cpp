#include "gui/OutfitListTable.h"

#include "ImGuiUtil.h"
#include "Translation.h"
#include "imgui.h"
#include <array>

namespace LIBC_NAMESPACE_DECL
{
    void OutfitListTable::Refresh()
    {
        m_clickIt    = m_uiData.GetOutfitList().cend();
        m_wantEditIt = m_uiData.GetOutfitList().end();
    }

    void OutfitListTable::Close()
    {
        m_clickIt    = m_uiData.GetOutfitList().cend();
        m_wantEditIt = m_uiData.GetOutfitList().end();
    }

    void OutfitListTable::Render(GuiContext &guiContext, ImVec2 childSize)
    {
        if (ImGuiUtil::BeginChild("$SkyOutSys_MCMHeader_OutfitList", childSize, ImGuiChildFlags_AutoResizeY))
        {
            RenderChildContent(guiContext);
        }
        // must out of BeginChild braces because when dock window, BeginChild will be return false;
        auto &outfits = m_uiData.GetOutfitList();
        if (m_clickIt != outfits.cend() && DeletePopup(m_clickIt))
        {
            m_clickIt = outfits.cend();
        }
        if (m_wantEditIt != outfits.end() && EditingPanel(m_wantEditIt))
        {
            m_wantEditIt = outfits.end();
        }
        ImGui::EndChild();
    }

    void OutfitListTable::RenderChildContent(GuiContext &guiContext)
    {
        static std::array<char, OUTFIT_NAME_MAX_BYTES> outfitNameBuf;
        ImGui::InputText("##CreateNewInput", outfitNameBuf.data(), outfitNameBuf.size());

        ImGui::BeginDisabled(outfitNameBuf[0] == '\0');
        if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
        {
            m_dataCoordinator.RequestCreateOutfit(outfitNameBuf.data());
        }

        ImGui::SameLine();
        if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
        {
            m_dataCoordinator.RequestCreateOutfitFromWorn(outfitNameBuf.data());
        }
        ImGui::EndDisabled();

        if (ImGuiUtil::Button("$SosGui_Refresh{$SkyOutSys_MCM_OutfitList}"))
        {
            m_dataCoordinator.RequestOutfitList();
        }
        auto &outfits = m_uiData.GetOutfitList();
        if (outfits.empty())
        {
            ImGuiUtil::TextScale("$SosGui_EmptyHint{$SkyOutSys_MCM_OutfitList}", HintFontSize());
        }
        static int selectedIdx = -1;

        if (m_outfitListTable.Begin())
        {
            m_outfitListTable.HeadersRow();

            int idx = 0;
            for (auto iterator = outfits.begin(); iterator != outfits.end(); ++iterator)
            {
                ImGuiUtil::PushIdGuard idGuard(idx);
                ImGui::TableNextRow();

                const auto &outfit     = *iterator;
                const auto &outfitName = outfit.GetName();
                ImGui::TableNextColumn();
                bool const isSelected = selectedIdx == idx;
                if (ImGui::Selectable(outfitName.data(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                {
                    selectedIdx = selectedIdx != idx ? idx : -1;
                }
                bool acceptEdit = false;
                if (OpenContextMenu(guiContext, iterator, acceptEdit))
                {
                    m_clickIt = iterator;
                }
                if (acceptEdit)
                {
                    OnAcceptEditOutfit(iterator);
                    m_wantEditIt = iterator;
                }
            }
            ImGui::EndTable();
        }
    }

    bool OutfitListTable::OpenContextMenu(GuiContext &guiContext, const SosUiData::OutfitConstIterator &selectIt,
                                          bool &acceptEdit)
    {
        if (ImGui::BeginPopupContextItem())
        {
            const auto &outfitName             = selectIt->GetName();
            auto        menuItemEditOutfitName = Translation::Translate("$SosGui_OpenOutfitEditPanel", outfitName);
            if (ImGui::MenuItem(menuItemEditOutfitName.c_str()))
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
                if (m_uiData.IsActorActiveOutfit(guiContext.editingActor, outfitName))
                {
                    if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOff"))
                    {
                        OnAcceptActiveOutfit(guiContext.editingActor, "");
                    }
                }
                if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOn"))
                {
                    OnAcceptActiveOutfit(guiContext.editingActor, outfitName);
                }
                ImGui::BeginDisabled(guiContext.editingState == StateType::None);
                {
                    if (ImGui::MenuItem("AutoSwitch: Use this outfit"))
                    {
                        OnAcceptOutfitForState(guiContext, selectIt->GetName());
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
        return false;
    }

    bool OutfitListTable::DeletePopup(const SosUiData::OutfitConstIterator &clickedIt)
    {
        bool isConfirmDelete = false;
        bool justClosed      = m_DeleteOutfitPopup.Render(clickedIt->GetName(), isConfirmDelete);
        if (isConfirmDelete)
        {
            m_dataCoordinator.RequestDeleteOutfit(clickedIt);
        }
        return justClosed;
    }

    void OutfitListTable::OnAcceptEditOutfit(const SosUiData::OutfitIterator &toEditIt)
    {
        m_dataCoordinator.RequestOutfitArmors(toEditIt);
        m_editPanel.ShowWindow(toEditIt->GetName());
    }

    void OutfitListTable::OnAcceptOutfitForState(GuiContext &guiContext, const std::string &outfitName)
    {
        m_dataCoordinator.RequestSetActorStateOutfit(guiContext.editingActor, guiContext.editingState, outfitName);
    }

    void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, const std::string &outfitName)
    {
        m_dataCoordinator.RequestActiveOutfit(editingActor, outfitName);
    }
}
