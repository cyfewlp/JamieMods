#include "SosGui.h"
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
    static constexpr int BASIC_TABLE_FLAGS = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;

    auto SosGui::ToggleShow() -> void
    {
        m_fShow = !m_fShow;

        if (m_fShow)
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ListActors);
            PapyrusEvent::GetInstance().CallGetAutoSwitchEnabled(RE::PlayerCharacter::GetSingleton());
            PapyrusEvent::GetInstance().CallGetOutfitList();
        }
    }

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
        SKSE::Translation::ParseTranslation("skyrimoutfitsystem");
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

    auto SosGui::DoRender() -> void
    {
        if (!m_fShow)
        {
            return;
        }
        ImGui::Begin("SosGuiOptions", &m_fShow);
        {
            bool fEnabled = SosUiData::GetInstance().IsEnabled();
            if (ImGui::Checkbox("Enabled", &fEnabled))
            {
                PapyrusEvent::GetInstance().CallSetEnabled(fEnabled);
            }

            RenderCharactersConfig();

            static bool enableQuickslots = false;

            ImGui::Checkbox("Quickslots", &enableQuickslots);

            ImGui::Button("Export to Json");
            ImGui::SameLine();
            ImGui::Button("Import from Json");

            RenderOutfitConfiguration();
        }
        ImGui::End();
    }

    constexpr void T(const char *key, std::string &result)
    {
        // TODO cache key
        SKSE::Translation::Translate(key, result);
    }

    constexpr auto T(const char *key) -> std::string
    {
        std::string result;
        SKSE::Translation::Translate(key, result);
        return result;
    }

    void SosGui::RenderCharactersConfig()
    {
        std::string key;
        T("$SkyOutSys_Text_ActiveActorHeader", key);
        ImGui::SeparatorText(key.c_str());

        T("$SkyOutSys_Text_AddActorSelection", key);
        static bool fShowNearNpcLis = false;
        if (ImGui::Checkbox(key.c_str(), &fShowNearNpcLis))
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
        if (ImGui::Button("Refresh CharactersList"))
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ListActors);
        }
        static int selectedIdx = -1;
        ImGui::BeginGroup();
        auto flags = BASIC_TABLE_FLAGS | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable;
        if (ImGui::BeginTable("CharactersList", 2, flags))
        {
            ImGui::TableSetupColumn("Actor##HeaderRow");
            ImGui::TableSetupColumn("Remove##HeaderRow");
            ImGui::TableHeadersRow();
            int idx = 0;
            for (const auto &actor : actors)
            {
                ImGui::PushID(idx);
                ImGui::TableNextRow();
                bool isSelected = selectedIdx == idx;
                if (isSelected)
                {
                    auto color = ImGui::GetColorU32(ImGuiCol_HeaderActive);
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, color);
                }

                ImGui::TableNextColumn();
                if (ImGui::Selectable(actor->GetName(), isSelected))
                {
                    selectedIdx = idx;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }

                ImGui::TableNextColumn();
                if (ImGui::Button("Remove"))
                {
                    PapyrusEvent::GetInstance().CallRemoveActor(actor);
                }
                ImGui::PopID();
                idx++;
            }
            ImGui::EndTable();
        }
        ImGui::EndGroup();

        if (selectedIdx >= 0 && selectedIdx < actors.size())
        {
            RenderLocationBasedAutoswitch(actors.at(selectedIdx));
        }
    }

    void SosGui::RenderLocationBasedAutoswitch(RE::Actor *currentActor)
    {
        bool        fAutoSwitchEnabled = SosUiData::GetInstance().IsAutoSwitchEnabled(currentActor);
        std::string key;
        T("$SkyOutSys_MCMHeader_Autoswitch", key);
        if (ImGui::Checkbox(key.c_str(), &fAutoSwitchEnabled))
        {
            PapyrusEvent::GetInstance().CallSetAutoSwitchEnabled(currentActor, fAutoSwitchEnabled);
        }
        // don't call GetAutoSwitchStateArray because it's result is static
        static std::array stateArray = {Combat, World,     WorldSnowy, WorldRainy, City,         CitySnowy,   CityRainy,
                                        Town,   TownSnowy, TownRainy,  Dungeon,    DungeonSnowy, DungeonRainy};
        if (fAutoSwitchEnabled)
        {
            std::string noneState;
            T("$SkyOutSys_AutoswitchEdit_None", noneState);

            auto flags = BASIC_TABLE_FLAGS | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable;
            if (ImGui::BeginTable("##AutoswitchStateList", 2, flags))
            {
                ImGui::TableSetupColumn("Location##Header");
                ImGui::TableSetupColumn("State##Header");
                ImGui::TableHeadersRow();

                int idx = 0;
                for (const auto &state : stateArray)
                {
                    ImGui::TableNextRow();
                    ImGui::PushID(idx);

                    T(std::format("$SkyOutSys_Text_Autoswitch{}", static_cast<int8_t>(state)).c_str(), key);
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", key.c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", noneState.c_str());

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
    }

    void SosGui::RenderOutfitConfiguration()
    {
        std::string key;
        T("$SkyOutSys_MCMHeader_OutfitList", key);

        ImGui::SeparatorText(key.c_str());
        // create
        static std::array<char, OUTFIT_NAME_MAX_BYTES> outfitNameBuf;
        ImGui::InputText("##CreateNewInput", outfitNameBuf.data(), outfitNameBuf.size());
        // TODO call AllowTextInput
        static bool        showErrorMessage = false;
        static std::string errorMessage;

        T("$SkyOutSys_OContext_New", key);
        if (ImGui::Button(key.c_str()))
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
        T("$SkyOutSys_OContext_NewFromWorn", key);
        if (ImGui::Button(key.c_str()))
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

        if (ImGui::Button("Refresh OutfitLists"))
        {
            PapyrusEvent::GetInstance().CallGetOutfitList();
        }
        auto        flags       = BASIC_TABLE_FLAGS | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendX;
        static int  selectedIdx = -1, prevSelectedIdx = -1;
        const auto &outfitList = SosUiData::GetInstance().GetOutfitList();
        if (ImGui::BeginTable("##OutfitLists", 1, flags))
        {
            ImGui::TableSetupColumn("Outfits##Header");
            ImGui::TableHeadersRow();
            int idx = 0;
            for (const auto &outfitName : outfitList)
            {
                ImGui::TableNextRow(ImGuiTableRowFlags_None);
                ImGui::TableNextColumn();
                bool isSelected = selectedIdx == idx;
                if (ImGui::Selectable(outfitName.data(), isSelected, ImGuiSelectableFlags_None))
                {
                    selectedIdx = idx;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
                idx++;
            }
            ImGui::EndTable();
        }
        if (selectedIdx != -1)
        {
            auto &outfitName = outfitList.at(selectedIdx);
            if (selectedIdx != prevSelectedIdx)
            {
                PapyrusEvent::GetInstance().CallGetOutfitArmors(outfitName.data());
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

    void SosGui::RenderOutfitArmors(const std::string_view &outfitName)
    {
        static auto flags = BASIC_TABLE_FLAGS | ImGuiTableFlags_Resizable;
        flags |= ImGuiTableFlags_Sortable;
        flags |= ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame;
        std::string key;
        T("$SkyOutSys_MCMHeader_OutfitSlots", key);
        if (ImGui::BeginTable(key.c_str(), 2, flags))
        {
            ImGui::TableSetupColumn("Slot##HeaderRow", ImGuiTableColumnFlags_DefaultSort);
            ImGui::TableSetupColumn("Armor##HeaderRow");
            ImGui::TableHeadersRow();
            auto slotArmors = SosUiData::GetInstance().GetOutfitBodySlotArmors();
            int  idx        = 0;
            if (slotArmors.contains(outfitName))
            {
                auto slotArmorList = slotArmors.at(outfitName);
                for (const auto &slotArmor : slotArmorList)
                {
                    ImGui::PushID(idx);
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    T(std::format("$SkyOutSys_BodySlot{}", slotArmor.first).c_str(), key);
                    ImGui::Text("%s", key.c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", slotArmor.second->GetName());

                    ImGui::PopID();
                    ++idx;
                }
            }
            ImGui::EndTable();
        }
    }

    void SosGui::RenderOutfitEditPanel(const std::string_view &outfitName)
    {
        static std::array outfitPolicy{T("$SkyOutSys_OEdit_AddFromCarried"), T("$SkyOutSys_OEdit_AddFromWorn"),
                                       T("$SkyOutSys_OEdit_AddByID"), T("$SkyOutSys_OEdit_AddFromList_Header")};

        std::string key;
        int         idx         = 0;
        static int  selectedIdx = 0;
        bool        fSelected   = false;
        ImGui::NewLine();
        for (const auto &policy : outfitPolicy)
        {
            ImGui::PushID(idx);

            ImGui::SameLine();
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
        T("$SkyOutSys_OEdit_AddFromList_Filter_Playable", key);
        if (ImGui::Checkbox(key.c_str(), &fFilterPlayable))
        {
            shouldUpdateCandidates = true;
        }
        if (selectedPolicy == OutfitAddPolicy_AddByID)
        {
            RenderOutfitAddPolicyById(outfitName, fFilterPlayable);
            return;
        }

        T("$SkyOutSys_OEdit_AddFromList_Filter_Name", key); // filter armor name and mod name
        ImGui::InputText(key.c_str(), filterStringBuf.data(), MAX_FILTER_ARMOR_NAME);
        if (!ImGui::GetIO().WantTextInput && ImGui::IsItemDeactivated())
        {
            shouldUpdateCandidates = true;
        }
        if (shouldUpdateCandidates)
        {
            UpdateArmorCandidates(filterStringBuf.data(), fFilterPlayable, selectedPolicy);
        }
        RenderOutfitAddPolicyByCandidates(outfitName);
    }

    void SosGui::RenderOutfitAddPolicyById(const std::string_view &outfitName, bool &fFilterPlayable)
    {
        static std::array<char, 32> formIdBuf;
        static char                *pEnd{};
        ImGui::Text("0x##FormIDPrefix");
        ImGui::SameLine();
        ImGui::InputText("##ArmorId", formIdBuf.data(), formIdBuf.size(),
                         ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal);
        auto               formId = std::strtoul(formIdBuf.data(), &pEnd, 16);
        RE::TESObjectARMO *armor  = nullptr;
        if (*pEnd == 0 && (armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(formId)) != nullptr)
        {
            ImGui::Text("%s", armor->GetName());
            ImGui::SameLine();
            if (fFilterPlayable && !!(armor->formFlags & RE::TESObjectARMO::RecordFlags::kNonPlayable))
            {
                ImGui::TextDisabled("Add");
            }
            else if (ImGui::Button("Add##ByFormId"))
            {
                PapyrusEvent::GetInstance().CallAddToOutfit(outfitName.data(), armor);
            }
        }
        ImGui::SameLine();
    }

    void SosGui::RenderOutfitAddPolicyByCandidates(const std::string_view &outfitName)
    {
        auto &outfitCandidates = SosUiData::GetInstance().GetArmorCandidatesCopy();
        auto  flags            = BASIC_TABLE_FLAGS | ImGuiTableFlags_Resizable;
        flags |= ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedSame;
        static int pageSize = 20, currentPage = 0, pageCount = 0;
        pageCount = outfitCandidates.size() / pageSize;
        if ((outfitCandidates.size() % pageSize) != 0)
        {
            pageCount += 1;
        }
        int  startIdx     = currentPage * pageSize;
        auto selectedSlot = RenderArmorSlotFilter();
        if (ImGui::BeginTable("##ArmorCandidates", 3, flags))
        {
            ImGui::TableSetupColumn("ArmorName##HeaderRow", ImGuiTableColumnFlags_DefaultSort);
            ImGui::TableSetupColumn("Slot##HeaderRow", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Add##HeaderRow", ImGuiTableColumnFlags_NoSort);
            ImGui::TableHeadersRow();

            int  idx   = 0;
            auto begin = outfitCandidates.begin() + startIdx;
            for (auto armorIter = begin; idx < pageSize && armorIter != outfitCandidates.end();)
            {
                auto *armor = *armorIter;
                if (selectedSlot != Slot::kNone && !armor->HasPartOf(selectedSlot))
                {
                    ++armorIter;
                    continue;
                }

                ImGui::PushID(idx);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%s", armor->GetName());

                ImGui::TableNextColumn();
                ImGui::Text("%d", static_cast<uint32_t>(armor->GetSlotMask()));

                ImGui::TableNextColumn();
                if (ImGui::Button("Add"))
                {
                    PapyrusEvent::GetInstance().CallAddToOutfit(outfitName.data(), armor);
                    armorIter = outfitCandidates.erase(armorIter);
                }
                else
                {
                    ++armorIter;
                }
                ImGui::PopID();
                ++idx;
            }
            ImGui::EndTable();
            if (currentPage > 0)
            {
                if (ImGui::Button("Previous"))
                {
                    currentPage -= 1;
                }
                ImGui::SameLine();
            }
            ImGui::Text("Page: %d/%d", currentPage + 1, pageCount);
            ImGui::SameLine();
            if (currentPage < pageCount - 1)
            {
                if (ImGui::Button("Next"))
                {
                    currentPage += 1;
                }
            }
        }
    }

    constexpr int BODY_SLOT_MIN = 29;

    auto SosGui::RenderArmorSlotFilter() -> Slot
    {
        static int  selectedIdx = 0;
        std::string key;

        if (ImGui::BeginCombo("##ArmorSlotFilter", ARMOR_SLOT_NAMES.at(selectedIdx), ImGuiComboFlags_None))
        {
            for (int idx = 0; idx <= 32; ++idx)
            {
                ImGui::PushID(idx);
                T(std::format("$SkyOutSys_BodySlot{}", idx + BODY_SLOT_MIN).c_str(), key);
                bool isSelected = selectedIdx == idx;
                if (ImGui::Selectable(key.c_str(), isSelected, ImGuiSelectableFlags_None))
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
            const auto form = list[idx];
            if (form && form->formType != RE::FormType::Armor)
            {
                continue;
            }
            auto armor = skyrim_cast<RE::TESObjectARMO *>(form);
            if (armor == nullptr || armor->templateArmor)
            {
                continue;
            }
            if (mustBePlayable && !!(armor->formFlags & RE::TESObjectARMO::RecordFlags::kNonPlayable))
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

    void SosGui::FilterArmorCandidates(const std::string_view           &filterString,
                                       std::vector<RE::TESObjectARMO *> &armorCandidates)
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

    auto SosGui::IsFilterArmor(const std::string_view &filterString, RE::TESObjectARMO *armor) -> bool
    {
        std::string armorName, modName;
        if (auto fullName = skyrim_cast<RE::TESFullName *>(armor); fullName != nullptr)
        {
            armorName.assign(fullName->GetFullName());
        }
        if (auto modFile = armor->GetFile(); modFile != nullptr)
        {
            modName.assign(modFile->GetFilename());
        }
        if (armorName.empty() || !armorName.contains(filterString) || !modName.contains(filterString))
        {
            return true;
        }
        return false;
    }
}
