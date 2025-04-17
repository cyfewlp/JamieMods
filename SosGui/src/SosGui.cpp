#include "SosGui.h"
#include "ImGuiUtil.h"
#include "common/config.h"
#include "common/log.h"
#include "coroutine.h"
#include "util/ImThemeLoader.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "util/ImGuiSettinsHandler.h"

#include <RE/C/ControlMap.h>
#include <RE/C/CursorMenu.h>
#include <RE/M/MenuCursor.h>
#include <RE/M/MiddleHighProcessData.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/R/Renderer.h>
#include <RE/S/SpellItem.h>
#include <RE/T/TESForm.h>
#include <RE/U/UI.h>
#include <REL/Relocation.h>
#include <SKSE/Impl/PCH.h>
#include <windows.h>

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

        // add our setting handler
        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName   = SKSE::PluginDeclaration::GetSingleton()->GetName().data();
        ini_handler.TypeHash   = ImHashStr(ini_handler.TypeName);
        ini_handler.ReadOpenFn = Setting::UiSettingReadOpenFn;
        ini_handler.ReadLineFn = Setting::UiSettingReadLineFn;
        ini_handler.WriteAllFn = Setting::UiSettingWriteAll;
        ini_handler.ClearAllFn = Setting::UiSettingClearAll;
        ini_handler.ApplyAllFn = Setting::UiSettingApplyAll;
        ImGui::AddSettingsHandler(&ini_handler);

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

        m_uiData.ExecuteUiTasks();
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
        log_debug("RequestOutfitList thread-id {}", std::this_thread::get_id());
        DoRefresh();
    }

    auto SosGui::DoRefresh() -> CoroutinePromise
    {
        co_await m_dataCoordinator.Refresh();
        m_outfitListTable.Refresh();
    }

    CoroutinePromise SosGui::operator<<(CoroutineTask &&task)
    {
        co_await task;
    }

    auto SosGui::DoRender() -> void
    {
        ImGui::Begin("SosGuiOptions", nullptr, ImGuiWindowFlags_NoNav);
        {
            ShowErrorMessages();
            bool fEnabled = m_uiData.IsEnabled();
            if (ImGuiUtil::CheckBox("$Enabled", &fEnabled))
            {
                *this << m_dataCoordinator.RequestEnable(fEnabled);
            }
            ImGui::Indent(2);
            ImGui::SameLine();
            auto key = Translation::Translate("$SosGui_Global_FontSize_Scale");

            if (ImGui::DragFloat(key.c_str(), &ImGui::GetIO().FontGlobalScale, 0.05F,
                                 Setting::UiSetting::FONT_SCALE_MIN, Setting::UiSetting::FONT_SCALE_MAX))
            {
                Setting::UiSetting::GetInstance()->globalFontScale = ImGui::GetIO().FontGlobalScale;
            }
            ThemeCombo();

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
            RenderLocationBasedAutoswitch(m_context.editingActor, childSize);
            ImGui::SameLine();

            m_outfitListTable.Render(m_context, childSize);
        }
        ImGui::End();
    }

    auto SosGui::ThemeCombo() -> void
    {
        auto   *settings    = Setting::UiSetting::GetInstance();
        int32_t themeIndex  = settings->selectedThemeIndex;
        using Loader        = ImThemeLoader::Loader;
        std::string preview = Loader::IsIndexInRange(themeIndex) ? Loader::g_availableThemes[themeIndex] : "";
        if (!ImGui::BeginCombo("Theme", preview.c_str()))
        {
            return;
        }
        size_t index = 0;
        for (const auto &themeName : Loader::g_availableThemes)
        {
            if (ImGui::Selectable(themeName.c_str(), false))
            {
                Loader::UseTheme(index);
                settings->selectedThemeIndex = index;
            }
            ++index;
        }
        ImGui::EndCombo();
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
            *this << m_dataCoordinator.RequestExportSettings();
        }
        ImGuiUtil::SetItemTooltip("$SkyOutSys_Text_Export");

        ImGui::SameLine();
        if (ImGuiUtil::Button("$SkyOutSys_Text_Import"))
        {
            *this << m_dataCoordinator.RequestImportSettings();
            Refresh();
        }
        ImGuiUtil::SetItemTooltip("$SkyOutSys_Desc_Import");
    }

    void SosGui::RefreshCurrentActorArmor()
    {
        if (m_context.editingActor != nullptr &&
            m_context.editingActor->GetActorRuntimeData().currentProcess != nullptr)
        {
            if (auto *currentProcess = m_context.editingActor->GetActorRuntimeData().currentProcess;
                currentProcess != nullptr)
            {
                currentProcess->Set3DUpdateFlag(RE::RESET_3D_FLAGS::kModel);
                m_context.editingActor->Update3DModel();
            }
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