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

        m_outfitArmorsTable.name = "##OutfitArmors";
        m_outfitArmorsTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
        m_outfitArmorsTable.flags |= ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame;
        m_outfitArmorsTable.headersRow = {ImGuiUtil::Translate("$SosGui_TableHeader_Slot"),
                                          ImGuiUtil::Translate("$$ARMOR")};

        m_armorCandidatesTable.name = "##ArmorCandidates";
        m_armorCandidatesTable.flags |= ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable;
        m_armorCandidatesTable.flags |= ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame;
        m_armorCandidatesTable.headersRow = {
            ImGuiUtil::Translate("$ARMOR"),
            ImGuiUtil::Translate("$SosGui_TableHeader_Slot"),
            ImGuiUtil::Translate("$Add"),
        };

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

    template <size_t Columns>
    bool RenderTable(ImTable<Columns> &imTable, std::function<bool(uint32_t &)> renderRow)
    {
        bool result = true;
        if (ImGui::BeginTable(imTable.name.c_str(), Columns, imTable.flags))
        {
            ImGui::PushID("HeaderRow");
            for (size_t idx = 0; idx < Columns; ++idx)
            {
                ImGui::TableSetupColumn(imTable.headersRow[idx].c_str());
            }
            ImGui::TableHeadersRow();

            for (uint32_t rowIdx = 0; rowIdx < imTable.rows;)
            {
                ImGui::PushID(rowIdx);
                ImGui::TableNextRow();
                if (!renderRow(rowIdx))
                {
                    result = false;
                    break;
                }
                ImGui::PopID();
            }
            ImGui::PopID();
            ImGui::EndTable();
        }
        return result;
    }

    template <size_t Columns>
    void RenderTable(ImTable<Columns> &imTable, std::function<void(int)> renderRow)
    {
        if (ImGui::BeginTable(imTable.name.c_str(), Columns, imTable.flags))
        {
            ImGui::PushID("HeaderRow");
            for (size_t idx = 0; idx < Columns; ++idx)
            {
                ImGui::TableSetupColumn(imTable.headersRow[idx].c_str());
            }
            ImGui::TableHeadersRow();

            for (uint32_t rowIdx = 0; rowIdx < imTable.rows; ++rowIdx)
            {
                ImGui::PushID(rowIdx);
                ImGui::TableNextRow();
                renderRow(rowIdx);
                ImGui::PopID();
            }
            ImGui::PopID();
            ImGui::EndTable();
        }
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
        RenderTable(m_charactersTable, [&actors, this](int rowIdx) {
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
            RenderTable(m_locationAutoSwitchTable, [](int idx) {
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
        static bool        showErrorMessage = false;
        static std::string errorMessage;

        if (ImGuiUtil::Button("$SkyOutSys_OContext_New"))
        {
            if (outfitNameBuf[0] == '\0')
            {
                showErrorMessage = true;
                errorMessage     = "Outfit name can't be empty";
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
                showErrorMessage = true;
                errorMessage     = "Outfit name can't be empty";
            }
            else
            {
                PapyrusEvent::GetInstance().CallCreateOutfit(outfitNameBuf.data(), true);
            }
        }

        if (showErrorMessage)
        {
            ImGui::TextColored({1.0F, 0.0F, 0.0F, 1.0F}, "%s", errorMessage.c_str());
            ImGui::SameLine();
            if (ImGui::Button("x##closeErrorMessage"))
            {
                showErrorMessage = false;
            }
        }

        if (ImGuiUtil::Button("$SosGui_Refresh{$SkyOutSys_MCM_OutfitList}"))
        {
            PapyrusEvent::GetInstance().CallGetOutfitList();
        }
        const auto &outfitList = SosUiData::GetInstance().GetOutfitList();
        ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
        if (outfitList.empty())
        {
            ImGuiUtil::Text("$SosGui_EmptyHint{$SkyOutSys_MCM_OutfitList}");
            ImGui::PopFontSize();
            return;
        }
        static int selectedIdx = -1, prevSelectedIdx = -1;
        if (selectedIdx == -1)
        {
            ImGuiUtil::Text("$SosGui_SelectHint{$SkyOutSys_MCM_OutfitList}");
        } else
        {
            ImGui::Text("");
        }
        ImGui::PopFontSize();

        m_outfitListTable.rows = outfitList.size();
        RenderTable(m_outfitListTable, [&outfitList](int idx) {
            auto outfitName = outfitList.at(idx);
            ImGui::TableNextColumn();
            bool const isSelected = selectedIdx == idx;
            if (ImGui::Selectable(outfitName.data(), isSelected, ImGuiSelectableFlags_None))
            {
                selectedIdx = selectedIdx != idx ? idx : -1;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        });

        if (selectedIdx != -1)
        {
            const auto &outfitName = outfitList.at(selectedIdx);
            if (selectedIdx != prevSelectedIdx)
            {
                PapyrusEvent::GetInstance().CallGetOutfitArmors(outfitName);
            }
            ImGui::BeginGroup();
            RenderOutfitArmors(outfitName);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            RenderOutfitEditPanel(outfitName);
            ImGui::EndGroup();
        }
        prevSelectedIdx = selectedIdx;
    }

    void SosGui::RenderOutfitArmors(const std::string &outfitName)
    {
        static auto empty_list    = std::vector<SosUiData::BodySlotArmor>();
        auto        slotArmors    = SosUiData::GetInstance().GetOutfitBodySlotArmors();
        auto        slotArmorList = slotArmors.contains(outfitName) ? slotArmors.at(outfitName) : empty_list;
        if (slotArmorList.empty())
        {
            ImGui::PushFontSize(ImGui::GetFontSize() * 1.2F);
            ImGuiUtil::Text("$SosGui_EmptyHint{$ARMOR}");
            ImGui::PopFontSize();
            return;
        }
        m_outfitArmorsTable.rows = slotArmorList.size();
        RenderTable(m_outfitArmorsTable, [&slotArmorList](int idx) {
            auto slotArmor = slotArmorList.at(idx);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGuiUtil::Text(std::format("$SkyOutSys_BodySlot{}", slotArmor.first));

            ImGui::TableNextColumn();
            ImGui::Text("%s", slotArmor.second->GetName());
        });
    }

    void SosGui::RenderOutfitEditPanel(const std::string &outfitName)
    {
        ImGui::BeginGroup();
        RenderOutfitAddPolicyByCandidates(outfitName);
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        RenderOutfitEditPanelPolicy(outfitName);
        ImGui::EndGroup();
    }

    void SosGui::RenderOutfitEditPanelPolicy(const std::string &outfitName)
    {
        static std::array outfitPolicy{ImGuiUtil::Translate("$SkyOutSys_OEdit_AddFromCarried"),
                                       ImGuiUtil::Translate("$SkyOutSys_OEdit_AddFromWorn"),
                                       ImGuiUtil::Translate("$SkyOutSys_OEdit_AddByID"),
                                       ImGuiUtil::Translate("$SkyOutSys_OEdit_AddFromList_Header")};

        int        idx         = 0;
        static int selectedIdx = 0;
        bool       fSelected   = false;
        ImGui::NewLine();
        for (const auto &policy : outfitPolicy)
        {
            ImGui::PushID(idx);

            if (ImGui::RadioButton(policy.c_str(), idx == selectedIdx))
            {
                selectedIdx = idx;
                fSelected   = true;
            }
            ImGui::PopID();
            idx++;
        }
        static std::array<char, MAX_FILTER_ARMOR_NAME> filterStringBuf;
        static bool                                    fFilterPlayable = false;

        const auto selectedPolicy = static_cast<OutfitAddPolicy>(selectedIdx);
        if (fSelected)
        {
            fSelected = false;
            if (selectedPolicy == OutfitAddPolicy_AddByID)
            {
                filterStringBuf[0] = '\0';
            }
        }

        bool shouldUpdateCandidates = fSelected;
        if (ImGuiUtil::CheckBox("$SkyOutSys_OEdit_AddFromList_Filter_Playable", &fFilterPlayable))
        {
            shouldUpdateCandidates = true;
        }
        if (selectedPolicy == OutfitAddPolicy_AddByID)
        {
            RenderOutfitAddPolicyById(outfitName, fFilterPlayable);
            return;
        }

        // filter armor name and mod name
        ImGuiUtil::InputText("$SkyOutSys_OEdit_AddFromList_Filter_Name", filterStringBuf);
        if (!ImGui::GetIO().WantTextInput && ImGui::IsItemDeactivated())
        {
            shouldUpdateCandidates = true;
        }
        if (shouldUpdateCandidates)
        {
            UpdateArmorCandidates(filterStringBuf.data(), fFilterPlayable, selectedPolicy);
        }
    }

    void SosGui::RenderOutfitAddPolicyById(const std::string &outfitName, const bool &fFilterPlayable)
    {
        static std::array<char, 32> formIdBuf;
        static char                *pEnd{};
        ImGui::PushID("AddByFormId");
        ImGui::Text("0x");
        ImGui::SameLine();
        ImGui::InputText("##InputArmorId", formIdBuf.data(), formIdBuf.size(),
                         ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal);
        auto   formId = std::strtoul(formIdBuf.data(), &pEnd, 16);
        Armor *armor  = nullptr;
        if (*pEnd == 0 && (armor = RE::TESForm::LookupByID<Armor>(formId)) != nullptr)
        {
            ImGui::Text("%s", armor->GetName());
            ImGui::SameLine();
            ImGui::BeginDisabled(fFilterPlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0);
            if (ImGuiUtil::Button("$Add"))
            {
                PapyrusEvent::GetInstance().CallAddToOutfit(outfitName, armor);
            }
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }

    void SosGui::RenderOutfitAddPolicyByCandidates(const std::string &outfitName)
    {
        static int  pageSize        = 20;
        static int  currentPage     = 0;
        static Slot preSelectedSlot = Slot::kNone;
        auto        selectedSlot    = RenderArmorSlotFilter();
        if (preSelectedSlot != selectedSlot)
        {
            currentPage     = 0;
            preSelectedSlot = selectedSlot;
            UpdateArmorCandidatesBySlot(preSelectedSlot);
        }
        auto &outfitCandidates = SosUiData::GetInstance().GetArmorCandidatesCopy();
        if (outfitCandidates.empty())
        {
            ImGuiUtil::TextScale("$SosGui_EmptyHint{$ARMOR}", 1.2F);
            return;
        }

        int  startIdx               = currentPage * pageSize;
        auto begin                  = outfitCandidates.begin() + startIdx;
        m_armorCandidatesTable.rows = pageSize;
        bool complete = RenderTable(m_armorCandidatesTable, [&begin, &outfitName, &outfitCandidates](uint32_t &rowIdx) {
            if (begin == outfitCandidates.end())
            {
                return false;
            }
            auto *armor = *begin;
            ImGui::TableNextColumn();
            ImGui::Text("%s", armor->GetName());

            ImGui::TableNextColumn();
            ImGui::Text("%d", static_cast<uint32_t>(armor->GetSlotMask()));

            ImGui::TableNextColumn();
            if (ImGuiUtil::Button("$Add"))
            {
                PapyrusEvent::GetInstance().CallAddToOutfit(outfitName, armor);
                begin = outfitCandidates.erase(begin);
            }
            else
            {
                ++begin;
            }
            ++rowIdx;
            return true;
        });

        ImGui::BeginDisabled(currentPage == 0);
        if (ImGuiUtil::Button("$SosGui_Table_PrevPage"))
        {
            currentPage -= 1;
        }
        ImGui::SameLine();
        ImGui::EndDisabled();

        ImGui::Text("%d", currentPage + 1);
        ImGui::SameLine();

        ImGui::BeginDisabled(!complete);
        if (ImGui::Button("$SosGui_Table_NextPage"))
        {
            currentPage += 1;
        }
        ImGui::EndDisabled();
    }

    constexpr int BODY_SLOT_MIN = 29;

    auto SosGui::RenderArmorSlotFilter() -> Slot
    {
        static int selectedIdx = 0;

        if (ImGui::BeginCombo("##ArmorSlotFilter", ARMOR_SLOT_NAMES.at(selectedIdx), ImGuiComboFlags_WidthFitPreview))
        {
            for (int idx = 0; idx <= 32; ++idx)
            {
                ImGui::PushID(idx);
                bool isSelected = selectedIdx == idx;
                if (ImGuiUtil::Selectable(std::format("$SkyOutSys_BodySlot{}", idx + BODY_SLOT_MIN), isSelected))
                {
                    selectedIdx = idx;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        return selectedIdx == 0 ? Slot::kNone : static_cast<Slot>(1 << (selectedIdx - 1));
    }

    void SosGui::UpdateArmorCandidates(const std::string_view &filterString, bool mustBePlayable,
                                       OutfitAddPolicy policy)
    {
        switch (policy)
        {
            case OutfitAddPolicy_AddFromCarried:
            case OutfitAddPolicy_AddFromWorn:
                PapyrusEvent::GetInstance().CallGetActorArmors(RE::PlayerCharacter::GetSingleton(), policy);
                break;
            case OutfitAddPolicy_AddByID:
                break;
            case OutfitAddPolicy_AddAny:
                UpdateArmorCandidatesForAny(filterString, mustBePlayable);
                break;
            default:
                break;
        }
    }

    void SosGui::UpdateArmorCandidatesForAny(const std::string_view &filterString, bool mustBePlayable)
    {
        auto       data   = RE::TESDataHandler::GetSingleton();
        auto      &list   = data->GetFormArray(RE::FormType::Armor);
        const auto size   = list.size();
        auto      &uiData = SosUiData::GetInstance();
        uiData.SetArmorCandidates({});
        auto &candidates = uiData.GetArmorCandidates();

        for (std::uint32_t idx = 0; idx < size; idx++)
        {
            auto *const form = list[idx];
            if (form != nullptr && form->formType != RE::FormType::Armor)
            {
                continue;
            }
            auto *armor = skyrim_cast<Armor *>(form);
            if (armor == nullptr || armor->templateArmor == nullptr)
            {
                continue;
            }
            if (mustBePlayable && (armor->formFlags & Armor::RecordFlags::kNonPlayable) != 0)
            {
                continue;
            }
            if (!IsFilterArmor(filterString, armor))
            {
                candidates.push_back(armor);
            }
        }
        uiData.ResetArmorCandidatesCopy();
    }

    void SosGui::UpdateArmorCandidatesBySlot(Slot slot)
    {
        const auto &candidates = SosUiData::GetInstance().GetArmorCandidates();
        auto       &copy       = SosUiData::GetInstance().GetArmorCandidatesCopy();
        copy.clear();
        for (const auto &candidate : candidates)
        {
            if (slot != Slot::kNone && !candidate->HasPartOf(slot))
            {
                continue;
            }
            copy.push_back(candidate);
        }
    }

    void SosGui::FilterArmorCandidates(const std::string_view &filterString, std::vector<Armor *> &armorCandidates)
    {
        if (filterString.empty())
        {
            return;
        }

        for (auto armorIter = armorCandidates.begin(); armorIter != armorCandidates.end();)
        {
            auto *armor = *armorIter;
            if (IsFilterArmor(filterString, armor))
            {
                armorIter = armorCandidates.erase(armorIter);
            }
            else
            {
                ++armorIter;
            }
        }
    }

    auto SosGui::IsFilterArmor(const std::string_view &filterString, Armor *armor) -> bool
    {
        std::string armorName;
        std::string modName;
        if (auto *fullName = skyrim_cast<RE::TESFullName *>(armor); fullName != nullptr)
        {
            armorName.assign(fullName->GetFullName());
        }
        if (auto *modFile = armor->GetFile(); modFile != nullptr)
        {
            modName.assign(modFile->GetFilename());
        }
        if (armorName.empty() || !armorName.contains(filterString) || !modName.contains(filterString))
        {
            return true;
        }
        return false;
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
