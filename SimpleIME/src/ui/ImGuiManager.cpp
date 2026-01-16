//
// Created by jamie on 2026/1/16.
//
#include "ui/ImGuiManager.h"

#include "common/imgui/ErrorNotifier.h"
#include "common/log.h"
#include "common/utils.h"
#include "ime/ImeController.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <RE/C/ControlMap.h>
#include <RE/C/CursorMenu.h>
#include <RE/M/MenuCursor.h>
#include <RE/U/UI.h>
#include <RE/U/UserEvents.h>
#include <d3d11.h>
#include <windef.h>

extern auto ImGui_ImplWin32_GetDpiScaleForHwnd(void *hwnd) -> float;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

void ImGuiManager::Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context, Settings &settings)
{
    if (g_initialized)
    {
        log_warn("ImGui already initialized!");
        return;
    }
    g_initialized = false;

    log_info("Initializing ImGui...");
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(device, context);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigNavMoveSetMousePos = false;

    settings.dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);
    InitFonts(settings);

    g_initialized = true;
    log_info("ImGui initialized!");
}

void ImGuiManager::InitFonts(const Settings &settings)
{
    ImFont *imFont = AddFonts(settings.resources.fontPathList);
    auto   &io     = ImGui::GetIO();
    if (imFont == nullptr)
    {
        ErrorNotifier::GetInstance().addError(
            "Can't load fonts! Try fallback to the default fonts settings...", ErrorMsg::Level::warning
        );
        io.Fonts->Clear();
        std::vector defaultFonts = {
            std::string(Settings::DEFAULT_MAIN_FONT_PATH), std::string(Settings::DEFAULT_EMOJI_FONT_PATH)
        };
        imFont = AddFonts(defaultFonts);
    }
    if (imFont == nullptr)
    {
        ErrorNotifier::GetInstance().addError(
            "Can't load fonts! Fallback to ImGui embedded font...", ErrorMsg::Level::warning
        );
        io.Fonts->Clear();
        imFont = io.Fonts->AddFontDefault();
    }

    io.FontDefault = imFont;
    auto iconFile  = CommonUtils::GetInterfaceFile(Settings::ICON_FILE);

    ImFontConfig cfg;
    cfg.OversampleH = 1;
    cfg.OversampleV = 1;
    cfg.PixelSnapH  = true;
    cfg.MergeMode   = true;
    if (!io.Fonts->AddFontFromFileTTF(iconFile.c_str(), 0.0f, &cfg))
    {
        ErrorNotifier::GetInstance().addError(
            std::format("Can't load icon font from {}", iconFile), ErrorMsg::Level::warning
        );
    }
}

auto ImGuiManager::AddFonts(const std::vector<std::string> &fontPaths) -> ImFont *
{
    ImFont *imFont = nullptr;
    if (fontPaths.empty())
    {
        return imFont;
    }

    auto &io = ImGui::GetIO();

    ImFontConfig cfg;
    cfg.OversampleH = 1;
    cfg.OversampleV = 1;
    cfg.PixelSnapH  = true;
    for (const auto &path : fontPaths)
    {
        cfg.MergeMode = imFont != nullptr;
        imFont        = io.Fonts->AddFontFromFileTTF(path.c_str(), 0.F, &cfg);
        if (imFont == nullptr)
        {
            break;
        }
    }
    return imFont;
}

/**
 * If Game cursor no showing/update, update ImGui cursor from system cursor pos
 */
void ImGuiManager::UpdateCursorPos()
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

void ImGuiManager::NewFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    UpdateCursorPos();
    ImGui::NewFrame();

    EnableTextInputIfNeed();
}

/**
 * @brief Notifies SkyrimSE to direct character events to ImeMenu.
 * We call @c ControlMap::AllowTextInput and @c ImeController::EnableIme to manage focus correctly.
 * This avoids the IME remaining enabled if the underlying menu
 * also has a text entry field.
 */
void ImGuiManager::EnableTextInputIfNeed()
{
    static bool fWantTextInput = false;
    bool        cWantTextInput = ImGui::GetIO().WantTextInput;
    const auto *imeManager     = ImeController::GetInstance();

    auto *controlMap = RE::ControlMap::GetSingleton();
    if (!fWantTextInput && cWantTextInput)
    {
        controlMap->AllowTextInput(true);
        imeManager->EnableIme(true);
        controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kMenu, false);
    }
    else if (fWantTextInput && !cWantTextInput)
    {
        controlMap->AllowTextInput(false);
        imeManager->EnableIme(false);
        controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kMenu, true);
    }
    fWantTextInput = cWantTextInput;
}

void ImGuiManager::EndFrame(Settings &settings)
{
    settings.appearance.fontSizeScale = ImGui::GetStyle().FontScaleMain;
}

void ImGuiManager::Render()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::Shutdown()
{
    if (g_initialized)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

}
}