#include "SosGui.h"
#include "ImGuiUtil.h"
#include "SosUiData.h"
#include "common/log.h"
#include "data/SosUiOutfit.h"
#include "gui/SosDataCoordinator.h"
#include "gui/Table.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <RE/R/Renderer.h>
#include <d3d11.h>
#include <format>

namespace LIBC_NAMESPACE_DECL
{
    auto SosGui::Init(const RE::BSGraphics::RendererData &renderData, HWND hWnd) -> bool
    {
        auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
        auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);

        log_info("Initializing ImGui...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        if (!ImGui_ImplWin32_Init(hWnd))
        {
            throw InitFail("ImGui initialization failed (Win32)");
        }

        if (!ImGui_ImplDX11_Init(device, context))
        {
            throw InitFail("ImGui initialization failed (DX11)");
        }

        RECT     rect = {0, 0, 0, 0};
        ImGuiIO &io   = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigNavMoveSetMousePos = false;
        ImGui::StyleColorsDark();
        ::GetClientRect(hWnd, &rect);
        io.DisplaySize = ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

        io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\simsun.ttc)", 14.0F, nullptr,
                                     io.Fonts->GetGlyphRangesChineseFull());

        ImGuiStyle &style = ImGui::GetStyle();
        if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
        {
            style.WindowRounding              = 0.0F;
            style.Colors[ImGuiCol_WindowBg].w = 1.0F;
        }

        log_info("ImGui initialized!");
        return true;
    }

    void SosGui::NewFrame()
    {
        if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
        {
            POINT cursorPos;
            if (ui->IsMenuOpen(RE::CursorMenu::MENU_NAME))
            {
                auto *menuCursor = RE::MenuCursor::GetSingleton();
                ImGui::GetIO().AddMousePosEvent(menuCursor->cursorPosX, menuCursor->cursorPosY);
            }
            else if (GetCursorPos(&cursorPos) != FALSE)
            {
                ImGui::GetIO().AddMousePosEvent(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
            }
        }
    }

    auto SosGui::Render() -> void
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        NewFrame();
        ImGui::NewFrame();

        TrySetAllowTextInput();

        DoRender();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void SosGui::TrySetAllowTextInput()
    {
        static bool fWantTextInput = false;
        const bool  cWantTextInput = ImGui::GetIO().WantTextInput;
        if (!fWantTextInput && cWantTextInput)
        {
            AllowTextInput(true);
        }
        else if (fWantTextInput && !cWantTextInput)
        {
            AllowTextInput(false);
        }

        fWantTextInput = cWantTextInput;
    }

    auto SosGui::Refresh() -> void
    {
        m_dataCoordinator.Refresh();
    }

    auto SosGui::DoRender() -> void
    {
        ImGui::Begin("SosGuiOptions", nullptr, ImGuiWindowFlags_NoNav);
        {
            ShowErrorMessages();
            bool fEnabled = m_uiData.IsEnabled();
            if (ImGuiUtil::CheckBox("$Enabled", &fEnabled))
            {
                m_dataCoordinator.RequestEnable(fEnabled);
            }
            ImGui::Indent(2);
            ImGui::SameLine();
            auto key = Translation::Translate("$SosGui_Global_FontSize_Scale");
            ImGui::DragFloat(key.c_str(), &ImGui::GetIO().FontGlobalScale, 0.05F, 0.5F, 5.0F);

            if (ImGui::Button("Refresh Armor"))
            {
                RefreshCurrentActorArmor();
            }

            RenderQuickSlotConfig();
            ImGui::SameLine();
            RenderExportOrImportSettings();
            RenderCharactersPanel();

            auto  &style     = ImGui::GetStyle();
            ImVec2 childSize = {(ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) * 0.5F, 0.0F};
            RenderLocationBasedAutoswitch(m_editingActor, childSize);
            ImGui::SameLine();
            RenderOutfitConfiguration(childSize);

            RenderEditingOutfit();

            RenderPopups();
        }
        ImGui::End();
    }

    void SosGui::ShowErrorMessages()
    {
        static ImVec4 RED_COLOR = ImColor(255, 0, 0, 255);

        auto &errorMessages = m_uiData.GetErrorMessages();
        auto  remainning    = MAX_ERROR_SHOW_COUNT;
        for (auto iter = errorMessages.begin(); remainning != 0 && iter != errorMessages.end(); --remainning)
        {
            ImGui::TextColored(RED_COLOR, "%s", (*iter).c_str());
            ImGui::SameLine();
            if (ImGui::Button("x"))
            {
                iter = errorMessages.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    void SosGui::RenderQuickSlotConfig()
    {
        bool quickSlotEnabled = m_uiData.IsQuickSlotEnabled();
        if (ImGuiUtil::CheckBox("$SkyOutSys_MCMHeader_Quickslots", &quickSlotEnabled))
        {
            if (!EnableQuickslot(quickSlotEnabled))
            {
                m_uiData.SetQuickSlotEnabled(false);
            }
        }

        ImGuiUtil::SetItemTooltip("$SkyOutSys_Desc_EnableQuickslots");
    }

    void SosGui::RenderExportOrImportSettings()
    {
        if (ImGuiUtil::Button("$SkyOutSys_Text_Export"))
        {
            m_dataCoordinator.RequestExportSettings();
        }
        ImGuiUtil::SetItemTooltip("$SkyOutSys_Text_Export");

        ImGui::SameLine();
        if (ImGuiUtil::Button("$SkyOutSys_Text_Import"))
        {
            m_dataCoordinator.RequestImportSettings();
            Refresh();
        }
        ImGuiUtil::SetItemTooltip("$SkyOutSys_Desc_Import");
    }

    void SosGui::RenderOutfitConfiguration(const ImVec2 &childSize)
    {
        if (ImGuiUtil::BeginChild("$SkyOutSys_MCMHeader_OutfitList", childSize, ImGuiChildFlags_AutoResizeY))
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
            auto &outfitMap = m_uiData.GetOutfitMap();
            if (outfitMap.empty())
            {
                ImGuiUtil::TextScale("$SosGui_EmptyHint{$SkyOutSys_MCM_OutfitList}", HintFontSize());
            }
            static int selectedIdx = -1;

            if (m_outfitListTable.Begin())
            {
                m_outfitListTable.HeadersRow();
                int idx = 0;
                for (auto &pair : outfitMap)
                {
                    ImGuiUtil::PushIdGuard idguard(idx);
                    ImGui::TableNextRow();

                    const auto &outfitName = pair.first;
                    ImGui::TableNextColumn();
                    bool const isSelected = selectedIdx == idx;
                    if (ImGui::Selectable(outfitName.data(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selectedIdx = selectedIdx != idx ? idx : -1;
                    }
                    RenderOutfitListContextMenu(pair.second);
                    ++idx;
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();
    }

    void SosGui::RenderEditingOutfit()
    {
        if (m_editingOutfit == nullptr)
        {
            return;
        }
        if (!m_guiOutfit.Render(*m_editingOutfit))
        {
            m_editingOutfit = nullptr;
        }
    }

    void SosGui::RenderOutfitListContextMenu(SosUiOutfit &outfit)
    {
        if (ImGui::BeginPopupContextItem())
        {
            m_selectedOutfit         = &outfit;
            const auto &outfitName   = outfit.GetName();
            auto        menuItemName = Translation::Translate("$SosGui_OpenOutfitEditPanel", outfitName);
            if (ImGui::MenuItem(menuItemName.c_str()))
            {
                OnAcceptEditingOutfit(outfit);
            }
            ImGui::Separator();
            ImGui::BeginDisabled(m_editingActor == nullptr);
            if (m_editingActor == nullptr)
            {
                ImGuiUtil::Text("$SosGui_SelectHint{$Characters}");
            }
            const auto *actorName = m_editingActor != nullptr ? m_editingActor->GetName() : "";
            ImGui::Text("%s", Translation::Translate("$SosGui_EditingActor", actorName).c_str());
            if (m_uiData.IsActorActiveOutfit(m_editingActor, outfitName))
            {
                if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOff"))
                {
                    ContextMenuSetActorActiveOutfit("");
                }
            }
            if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOn"))
            {
                ContextMenuSetActorActiveOutfit(outfitName);
            }
            ImGui::BeginDisabled(m_editingAutoSwitchState == StateType::None);
            if (ImGui::MenuItem("AutoSwitch: Use this outfit"))
            {
                OnAcceptOutfitForState(outfit.GetName());
            }
            ImGui::EndDisabled();
            ImGui::EndDisabled();
            if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_Delete"))
            {
                m_DeleteOutfitPopup.Open();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void SosGui::ContextMenuSetActorActiveOutfit(std::string outfitName)
    {
        ImGui::CloseCurrentPopup();
        m_dataCoordinator.RequestActiveOutfit(m_editingActor, outfitName, [this]() {
            RefreshCurrentActorArmor();
        });
    }

    void SosGui::RenderPopups()
    {
        if (m_selectedOutfit != nullptr)
        {
            if (m_DeleteOutfitPopup.Render(m_selectedOutfit->GetName()))
            {
                m_dataCoordinator.RequestDeleteOutfit(m_selectedOutfit->GetName());
            }
        }
        if (m_DeleteOutfitPopup.IsLastClosed())
        {
            m_selectedOutfit = nullptr;
        }
    }

    void SosGui::RefreshCurrentActorArmor()
    {
        if (m_editingActor != nullptr && m_editingActor->GetActorRuntimeData().currentProcess != nullptr)
        {
            if (auto *currentProcess = m_editingActor->GetActorRuntimeData().currentProcess; currentProcess != nullptr)
            {
                currentProcess->Set3DUpdateFlag(RE::RESET_3D_FLAGS::kModel);
                m_editingActor->Update3DModel();
            }
        }
    }

    void SosGui::OnAcceptEditingOutfit(SosUiOutfit &outfit)
    {
        m_editingOutfit = &outfit;
        m_dataCoordinator.RequestOutfitArmors(outfit.GetName());
        m_guiOutfit.ShowWindow(outfit.GetName());
    }

    void SosGui::OnAcceptOutfitForState(const std::string &outfitName)
    {
        if (m_editingActor == nullptr || m_editingAutoSwitchState == StateType::None)
        {
            return;
        }

        m_dataCoordinator.RequestSetActorStateOutfit(m_editingActor, m_editingAutoSwitchState, outfitName);
    }

    auto SosGui::EnableQuickslot(bool enable) -> bool
    {
        const auto &player = RE::PlayerCharacter::GetSingleton();
        auto       *spell  = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID);
        if (spell != nullptr)
        {
            enable ? player->AddSpell(spell) : player->RemoveSpell(spell);
            return true;
        }
        return false;
    }

    void SosGui::AllowTextInput(bool allow)
    {
        AllowTextInput1(RE::ControlMap::GetSingleton(), allow);
    }

    void SosGui::AllowTextInput1(RE::ControlMap *controlMap, bool allow)
    {
        using func_t = decltype(&SosGui::AllowTextInput1);
        static REL::Relocation<func_t> func{RELOCATION_ID(67252, 68552)};
        func(controlMap, allow);
    }
}
