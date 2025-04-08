#include "SosGui.h"
#include "ImGuiUtil.h"
#include "PapyrusEvent.h"
#include "SosOutfit.h"
#include "SosUiData.h"
#include "common/log.h"
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

    void SosGui::InitTables()
    {
        m_outfitListTable.name = "##OutfitLists";
        m_outfitListTable.flags |= ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_SizingStretchProp;
        m_outfitListTable.headersRow = {Translation::Translate("$SkyOutSys_MCM_OutfitList")};

        m_locationAutoSwitchTable.name = "##AutoswitchStateList";
        m_locationAutoSwitchTable.flags |= ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable;
        m_locationAutoSwitchTable.flags |= ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_SizingStretchProp;
        m_locationAutoSwitchTable.headersRow = {
            Translation::Translate("$SosGui_TableHeader_Location"),
            Translation::Translate("$SosGui_TableHeader_Location_State"),
        };

        m_charactersTable.name = "##CharactersTable";
        m_charactersTable.flags |=
            ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable;
        m_charactersTable.headersRow = {
            Translation::Translate("$Characters"),
            Translation::Translate("$Delete"),
            Translation::Translate("$SosGui_TableHeader_ActiveOutfit"),
        };
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
        PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ListActors);
        PapyrusEvent::GetInstance().CallGetAutoSwitchEnabled(RE::PlayerCharacter::GetSingleton());
        PapyrusEvent::GetInstance().CallGetOutfitList();

        SosUiData::GetInstance().SetQuickSlotEnabled(HasQuickslotSpell());
    }

    auto SosGui::DoRender() -> void
    {
        ImGui::Begin("SosGuiOptions", nullptr, ImGuiWindowFlags_NoNav);
        {
            bool fEnabled = SosUiData::GetInstance().IsEnabled();
            if (ImGuiUtil::CheckBox("$Enabled", &fEnabled))
            {
                PapyrusEvent::GetInstance().CallSetEnabled(fEnabled);
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
            RenderOutfitConfiguration();
            RenderEditingOutfit();
        }
        ImGui::End();
    }

    void SosGui::RenderQuickSlotConfig()
    {
        bool quickSlotEnabled = SosUiData::GetInstance().IsQuickSlotEnabled();
        if (ImGuiUtil::CheckBox("$SkyOutSys_MCMHeader_Quickslots", &quickSlotEnabled))
        {
            if (!EnableQuickslot(quickSlotEnabled))
            {
                SosUiData::GetInstance().SetQuickSlotEnabled(false);
            }
        }

        ImGuiUtil::SetItemTooltip("$SkyOutSys_Desc_EnableQuickslots");
    }

    void SosGui::RenderExportOrImportSettings()
    {
        if (ImGuiUtil::Button("$SkyOutSys_Text_Export"))
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ExportSettings);
        }
        ImGuiUtil::SetItemTooltip("$SkyOutSys_Text_Export");

        ImGui::SameLine();
        if (ImGuiUtil::Button("$SkyOutSys_Text_Import"))
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ImportSettings);
            Refresh();
        }
        ImGuiUtil::SetItemTooltip("$SkyOutSys_Desc_Import");
    }

    void SosGui::RenderOutfitConfiguration()
    {
        if (ImGuiUtil::BeginChild("$SkyOutSys_MCMHeader_OutfitList", {}, ImGuiChildFlags_AutoResizeY))
        {
            static std::array<char, OUTFIT_NAME_MAX_BYTES> outfitNameBuf;
            ImGui::InputText("##CreateNewInput", outfitNameBuf.data(), outfitNameBuf.size());

            ImGui::BeginDisabled(outfitNameBuf[0] == '\0');
            if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
            {
                PapyrusEvent::GetInstance().CallCreateOutfit(outfitNameBuf.data(), false);
            }

            ImGui::SameLine();
            if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
            {
                PapyrusEvent::GetInstance().CallCreateOutfit(outfitNameBuf.data(), true);
            }
            ImGui::EndDisabled();

            if (ImGuiUtil::Button("$SosGui_Refresh{$SkyOutSys_MCM_OutfitList}"))
            {
                PapyrusEvent::GetInstance().CallGetOutfitList();
            }
            auto &outfitMap = SosUiData::GetInstance().GetOutfitMap();
            ImGui::PushFontSize(HintFontSize());
            if (outfitMap.empty())
            {
                ImGuiUtil::Text("$SosGui_EmptyHint{$SkyOutSys_MCM_OutfitList}");
            }
            static int selectedIdx     = -1;
            ImGuiUtil::Text(selectedIdx == -1 ? "$SosGui_SelectHint{$SkyOutSys_MCM_OutfitList}" : "");
            ImGui::PopFontSize();

            m_outfitListTable.rows = outfitMap.size();
            if (BeginTable(m_outfitListTable))
            {
                TableHeadersRow(m_outfitListTable);
                int idx = 0;
                for (auto &pair : outfitMap)
                {
                    ImGui::PushID(idx);
                    ImGui::TableNextRow();

                    const auto &outfitName = pair.first;
                    ImGui::TableNextColumn();
                    bool const isSelected = selectedIdx == idx;
                    if (ImGui::Selectable(outfitName.data(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        OnSelectOutfit(pair.second, isSelected);
                        selectedIdx = selectedIdx != idx ? idx : -1;
                    }
                    RenderOutfitListContextMenu(outfitName);
                    ImGui::PopID();
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
        m_editingOutfit->Render();
    }

    void SosGui::RenderOutfitListContextMenu(const std::string &outfitName)
    {
        if (ImGui::BeginPopupContextItem())
        {
            if (m_editingActor == nullptr)
            {
                ImGuiUtil::Text("$SosGui_SelectHint{$Characters}");
            }
            else
            {
                auto       &sosUiData = SosUiData::GetInstance();
                const auto *actorName = m_editingActor->GetName();
                auto        templateS = Translation::Translate("$SosGui_ContextMenu_EditingActor");
                ImGui::Text("%s", std::vformat(templateS, std::make_format_args(actorName)).c_str());
                if (sosUiData.HasActiveOutfit(m_editingActor))
                {
                    if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOff"))
                    {
                        ContextMenuSetActorActiveOutfit("");
                    }
                }
                else
                {
                    if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_ToggleOn"))
                    {
                        ContextMenuSetActorActiveOutfit(outfitName);
                    }
                }
                if (ImGuiUtil::MenuItem("$SkyOutSys_OContext_Delete"))
                {
                    ContextMenuDeleteOutfit(outfitName);
                }
            }
            if (ImGui::MenuItem("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void SosGui::ContextMenuSetActorActiveOutfit(const std::string &outfitName)
    {
        PapyrusEvent::GetInstance().CallActiveOutfit(m_editingActor, outfitName);
        SosUiData::GetInstance().SetActorActiveOutfit(m_editingActor, outfitName);
        ImGui::CloseCurrentPopup();
        RefreshCurrentActorArmor();
    }

    void SosGui::ContextMenuDeleteOutfit(const std::string &outfitName)
    {
        PapyrusEvent::GetInstance().CallDeleteOutfit(outfitName);
        SosUiData::GetInstance().DeleteOutfit(outfitName);
        ImGui::CloseCurrentPopup();
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

    void SosGui::OnSelectOutfit(SosOutfit &outfit, bool prevSelectState)
    {
        if (prevSelectState)
        {
            m_editingOutfit = nullptr;
        }
        else
        {
            m_editingOutfit = &outfit;
            PapyrusEvent::GetInstance().CallGetOutfitArmors(outfit.GetName());
            outfit.ShowOutfitWindow(true);
        }
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

    auto SosGui::HasQuickslotSpell() -> bool
    {
        const auto &player = RE::PlayerCharacter::GetSingleton();
        auto       *spell  = RE::TESForm::LookupByEditorID<RE::SpellItem>(SOS_SPELL_EDITOR_ID);
        if (spell != nullptr)
        {
            return player->HasSpell(spell);
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
