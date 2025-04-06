#include "SosGui.h"
#include "ImGuiUtil.h"
#include "PapyrusEvent.h"
#include "SosUiData.h"
#include "common/log.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <RE/R/Renderer.h>
#include <d3d11.h>

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
        m_outfitListTable.name       = "##OutfitLists";
        m_outfitListTable.headersRow = {ImGuiUtil::Translate("$SkyOutSys_MCM_OutfitList")};

        m_locationAutoSwitchTable.name = "##AutoswitchStateList";
        m_locationAutoSwitchTable.flags |= ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable;
        m_locationAutoSwitchTable.headersRow = {
            ImGuiUtil::Translate("$SosGui_TableHeader_Location"),
            ImGuiUtil::Translate("$SosGui_TableHeader_Location_State"),
        };

        m_charactersTable.name = "##CharactersTable";
        m_charactersTable.flags |= ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable;
        m_charactersTable.headersRow = {ImGuiUtil::Translate("$Characters"), ImGuiUtil::Translate("$Delete")};
    }

    void SosGui::ShowErrorMessage()
    {
        if (!m_fShowMessage)
        {
            return;
        }
        ImGui::NewLine();
        static ImVec4 ERROR_COLOR = ImColor(255, 0, 0, 255);
        ImGui::TextColored(ERROR_COLOR, "%s", m_errorMessage.c_str());
        ImGui::SameLine();
        if (ImGui::Button("x##CancelErrorMessage"))
        {
            m_fShowMessage = false;
        }
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
        bool        cWantTextInput = ImGui::GetIO().WantTextInput;
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

    auto SosGui::DoRender() -> void
    {
        ImGui::Begin("SosGuiOptions", nullptr, ImGuiWindowFlags_NoNav);
        {
            bool fEnabled = SosUiData::GetInstance().IsEnabled();
            if (ImGuiUtil::CheckBox("$Enabled", &fEnabled))
            {
                PapyrusEvent::GetInstance().CallSetEnabled(fEnabled);
            }

            RenderQuickSlotConfig();
            RenderCharactersConfig();

            ImGuiUtil::Button("$SkyOutSys_Text_Export");
            ImGuiUtil::SetItemTooltip("$SkyOutSys_Text_Export");

            ImGui::SameLine();
            ImGuiUtil::Button("$SkyOutSys_Text_Import");
            ImGuiUtil::SetItemTooltip("$SkyOutSys_Desc_Import");

            RenderOutfitConfiguration();
        }
        ImGui::End();
    }

    void SosGui::RenderCharactersConfig()
    {
        ImGuiUtil::SeparatorText("$SkyOutSys_Text_ActiveActorHeader");

        static bool fShowNearNpcLis = false;
        if (ImGuiUtil::CheckBox("$SkyOutSys_Text_AddActorSelection", &fShowNearNpcLis))
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ActorNearPC);
        }
        if (fShowNearNpcLis)
        {
            static int  selectedIdx = 0;
            const auto &nearActors  = SosUiData::GetInstance().GetNearActors();
            if (!nearActors.empty())
            {
                if (ImGui::BeginCombo("##ActorNearPC", nearActors.at(selectedIdx)->GetName()))
                {
                    int idx = 0;
                    for (const auto &nearActor : nearActors)
                    {
                        if (ImGui::Selectable(nearActor->GetName(), idx == selectedIdx, ImGuiSelectableFlags_None))
                        {
                            selectedIdx = idx;
                            PapyrusEvent::GetInstance().CallAddActor(nearActors.at(idx));
                        }
                        if (selectedIdx == idx)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                        idx++;
                    }
                    ImGui::EndCombo();
                }
            }
        }
        RenderCharactersList();
    }

    void SosGui::RenderCharactersList()
    {
        const auto &actors = SosUiData::GetInstance().GetActors();
        if (ImGuiUtil::Button("$SosGui_Refresh{$Characters}"))
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ListActors);
        }
        static int selectedIdx = -1;
        ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
        if (selectedIdx == -1)
        {
            ImGuiUtil::Text("$SosGui_SelectHint{$Characters}");
        }
        else
        {
            ImGui::Text("");
        }
        ImGui::PopFontSize();

        ImGui::BeginGroup();
        m_charactersTable.rows = actors.size();
        ImGuiUtil::RenderTable(m_charactersTable, [&actors, this](int rowIdx) {
            auto       actor      = actors.at(rowIdx);
            bool const isSelected = selectedIdx == rowIdx;
            if (isSelected)
            {
                auto color = ImGui::GetColorU32(ImGuiCol_HeaderActive);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, color);
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable(actor->GetName(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
            {
                selectedIdx = selectedIdx != rowIdx ? rowIdx : -1;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }

            ImGui::TableNextColumn();

            if (ImGui::Button(m_charactersTable.headersRow[1].c_str()))
            {
                PapyrusEvent::GetInstance().CallRemoveActor(actor);
            }
        });
        ImGui::EndGroup();

        if (selectedIdx >= 0 && selectedIdx < actors.size())
        {
            RenderLocationBasedAutoswitch(actors.at(selectedIdx));
        }
    }

    void SosGui::RenderLocationBasedAutoswitch(RE::Actor *currentActor)
    {
        bool fAutoSwitchEnabled = SosUiData::GetInstance().IsAutoSwitchEnabled(currentActor);
        if (ImGuiUtil::CheckBox("$SkyOutSys_MCMHeader_Autoswitch", &fAutoSwitchEnabled))
        {
            PapyrusEvent::GetInstance().CallSetAutoSwitchEnabled(currentActor, fAutoSwitchEnabled);
        }
        // don't call GetAutoSwitchStateArray because it's result is static
        static constexpr std::array stateArray = {Combat,    World,        WorldSnowy,  WorldRainy, City,
                                                  CitySnowy, CityRainy,    Town,        TownSnowy,  TownRainy,
                                                  Dungeon,   DungeonSnowy, DungeonRainy};
        m_locationAutoSwitchTable.rows         = stateArray.size();
        if (fAutoSwitchEnabled)
        {
            static std::string noneState = ImGuiUtil::Translate("$SkyOutSys_AutoswitchEdit_None");
            ImGuiUtil::RenderTable(m_locationAutoSwitchTable, [](int idx) {
                auto state = stateArray.at(idx);
                ImGui::TableNextColumn();
                ImGuiUtil::Text(std::format("$SkyOutSys_Text_Autoswitch{}", static_cast<int8_t>(state)));

                ImGui::TableNextColumn();
                ImGui::Text("%s", noneState.c_str());
            });
        }
    }

    void SosGui::RenderOutfitConfiguration()
    {
        ImGuiUtil::SeparatorText("$SkyOutSys_MCMHeader_OutfitList");

        // create
        static std::array<char, OUTFIT_NAME_MAX_BYTES> outfitNameBuf;
        ImGui::InputText("##CreateNewInput", outfitNameBuf.data(), outfitNameBuf.size());
        if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
        {
            if (outfitNameBuf[0] == '\0')
            {
                AddErrorMessage("Outfit name can't be empty");
            }
            else
            {
                PapyrusEvent::GetInstance().CallCreateOutfit(outfitNameBuf.data(), false);
            }
        }
        ImGui::SameLine();
        if (ImGuiUtil::Button("$SkyOutSys_OContext_NewFromWorn"))
        {
            if (outfitNameBuf[0] == '\0')
            {
                AddErrorMessage("Outfit name can't be empty");
            }
            else
            {
                PapyrusEvent::GetInstance().CallCreateOutfit(outfitNameBuf.data(), true);
            }
        }
        ShowErrorMessage();

        if (ImGuiUtil::Button("$SosGui_Refresh{$SkyOutSys_MCM_OutfitList}"))
        {
            PapyrusEvent::GetInstance().CallGetOutfitList();
        }
        auto &outfitMap = SosUiData::GetInstance().GetOutfitMap();
        ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
        if (outfitMap.empty())
        {
            ImGuiUtil::Text("$SosGui_EmptyHint{$SkyOutSys_MCM_OutfitList}");
            ImGui::PopFontSize();
            return;
        }
        static std::string selectedOutfit;
        static int         selectedIdx     = -1;
        static int         prevSelectedIdx = -1;
        if (selectedIdx == -1)
        {
            ImGuiUtil::Text("$SosGui_SelectHint{$SkyOutSys_MCM_OutfitList}");
        }
        else
        {
            ImGui::Text("");
        }
        ImGui::PopFontSize();

        auto pair              = outfitMap.begin();
        m_outfitListTable.rows = outfitMap.size();
        ImGuiUtil::RenderTable(m_outfitListTable, [&pair](uint32_t &idx) {
            auto outfitName = pair->first;
            ImGui::TableNextColumn();
            bool const isSelected = selectedIdx == idx;
            if (ImGui::Selectable(outfitName.data(), isSelected, ImGuiSelectableFlags_None))
            {
                selectedIdx = selectedIdx != idx ? idx : -1;
                selectedOutfit.assign(outfitName);
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ++idx;
            ++pair;
            return true;
        });

        if (selectedIdx != -1)
        {
            if (selectedIdx != prevSelectedIdx)
            {
                PapyrusEvent::GetInstance().CallGetOutfitArmors(selectedOutfit);
            }
            outfitMap.at(selectedOutfit).Render();
        }
        prevSelectedIdx = selectedIdx;
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
