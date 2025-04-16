#include "gui/OutfitListTable.h"

#include "GuiContext.h"
#include "ImGuiUtil.h"
#include "Translation.h"
#include "imgui.h"

#include <array>
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
        if (ImGuiUtil::BeginChild("$SkyOutSys_MCMHeader_OutfitList", childSize, ImGuiChildFlags_AutoResizeY))
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

    void OutfitListTable::RenderChildContent(GuiContext &guiContext)
    {
        static std::array<char, OUTFIT_NAME_MAX_BYTES> outfitNameBuf;
        ImGui::InputText("##CreateNewInput", outfitNameBuf.data(), outfitNameBuf.size());

        ImGui::BeginDisabled(outfitNameBuf[0] == '\0');
        if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
        {
            *this << m_dataCoordinator.RequestCreateOutfit(outfitNameBuf.data());
        }

        ImGui::SameLine();
        if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
        {
            *this << m_dataCoordinator.RequestCreateOutfitFromWorn(outfitNameBuf.data());
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
            for (auto &outfitPair : outfits)
            {
                ImGuiUtil::PushIdGuard idGuard(idx);
                ImGui::TableNextRow();

                const auto &outfit     = outfitPair.second;
                const auto &outfitName = outfit.GetName();
                ImGui::TableNextColumn();
                bool const isSelected = selectedIdx == idx;
                if (ImGui::Selectable(outfitName.data(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                {
                    selectedIdx = selectedIdx != idx ? idx : -1;
                }
                bool acceptEdit = false;
                if (OpenContextMenu(guiContext, outfitPair.second.GetName(), acceptEdit))
                {
                    m_click = std::make_pair(outfitPair.first, &outfitPair.second);
                }
                if (acceptEdit)
                {
                    auto pair = std::make_pair(outfitPair.first, &outfitPair.second);
                    OnAcceptEditOutfit(pair);
                    m_wantEdit = pair;
                }
            }
            ImGui::EndTable();
        }
    }

    bool OutfitListTable::OpenContextMenu(GuiContext &guiContext, const std::string &outfitName, bool &acceptEdit)
    {
        if (!ImGui::BeginPopupContextItem())
        {
            return false;
        }
        auto menuItemEditOutfitName = Translation::Translate("$SosGui_OpenOutfitEditPanel", outfitName);
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
            *this << m_dataCoordinator.RequestDeleteOutfit(clicked);
        }
        return justClosed;
    }

    void OutfitListTable::OnAcceptEditOutfit(const SosUiData::OutfitPair &wantEdit)
    {
        *this << m_dataCoordinator.RequestOutfitArmors(wantEdit);
        *this << m_dataCoordinator.RequestOutfitSlotPolicy(wantEdit);
        m_editPanel.ShowWindow(wantEdit.second->GetName());
    }

    void OutfitListTable::OnAcceptOutfitForState(GuiContext &guiContext, const std::string &outfitName)
    {
        *this << m_dataCoordinator.RequestSetActorStateOutfit(guiContext.editingActor, guiContext.editingState,
                                                              outfitName);
    }

    void OutfitListTable::OnAcceptActiveOutfit(RE::Actor *editingActor, const std::string &outfitName)
    {
        *this << m_dataCoordinator.RequestActiveOutfit(editingActor, outfitName);
    }
}
