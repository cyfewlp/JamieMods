//
// Created by jamie on 2026/1/16.
//
#include "ui/ImGuiManager.h"

#include "common/imgui/ErrorNotifier.h"
#include "common/imgui/Material3.h"
#include "common/log.h"
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

    settings.state.dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);

    g_initialized = true;
    log_info("ImGui initialized!");
}

auto ImGuiManager::AddPrimaryFont(const std::vector<std::string> &fontsPathList) -> ImFont *
{
    ImFont *imFont = AddFonts(fontsPathList);
    auto   &io     = ImGui::GetIO();
    if (imFont == nullptr)
    {
        ErrorNotifier::GetInstance().addError(
            "Can't load fonts! Try fallback to the default fonts settings...", ErrorMsg::Level::warning
        );
        io.Fonts->Clear();
        auto defaultFonts =
            std::vector{std::string(Settings::DEFAULT_MAIN_FONT_PATH), std::string(Settings::DEFAULT_EMOJI_FONT_PATH)};
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
    return imFont;
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

auto ImGuiManager::AddFont(const std::string &filePath) -> ImFont *
{
    return ImGui::GetIO().Fonts->AddFontFromFileTTF(filePath.c_str());
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
    // settings.appearance.fontSizeScale = ImGui::GetStyle().FontScaleMain;
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

void ImGuiManager::ApplyM3Theme(ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGuiStyle &style = ImGui::GetStyle();

    auto &colors = m3Styles.Colors();

    style.Colors[ImGuiCol_Text]             = colors[ImGuiEx::M3::ContentToken::onSurface];
    style.Colors[ImGuiCol_TitleBg]          = colors[ImGuiEx::M3::SurfaceToken::surfaceContainer];
    style.Colors[ImGuiCol_TitleBgActive]    = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TitleBgCollapsed] = colors[ImGuiEx::M3::SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_WindowBg] = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_ChildBg]  = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_PopupBg]  = colors[ImGuiEx::M3::SurfaceToken::primaryContainer];

    style.Colors[ImGuiCol_FrameBg]       = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_FrameBgActive] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Pressed(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );
    style.Colors[ImGuiCol_FrameBgHovered] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Hovered(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );
    style.Colors[ImGuiCol_Border]       = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_BorderShadow] = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_SliderGrab] = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_SliderGrabActive] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Pressed(colors[ImGuiEx::M3::ContentToken::onPrimary]);

    style.Colors[ImGuiCol_Button] = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_ButtonHovered] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Hovered(colors[ImGuiEx::M3::ContentToken::onPrimary]);
    style.Colors[ImGuiCol_ButtonActive] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Pressed(colors[ImGuiEx::M3::ContentToken::onPrimary]);

    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = colors[ImGuiEx::M3::SurfaceToken::outline];
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = colors[ImGuiEx::M3::SurfaceToken::primary];

    style.Colors[ImGuiCol_MenuBarBg] = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh];

    style.Colors[ImGuiCol_Header] = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_HeaderHovered] =
        colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh].Hovered(colors[ImGuiEx::M3::ContentToken::onSurface]);
    style.Colors[ImGuiCol_HeaderActive] =
        colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh].Pressed(colors[ImGuiEx::M3::ContentToken::onSurface]);

    style.Colors[ImGuiCol_Separator] = colors[ImGuiEx::M3::SurfaceToken::secondary];
    style.Colors[ImGuiCol_SeparatorHovered] =
        colors[ImGuiEx::M3::SurfaceToken::secondary].Hovered(colors[ImGuiEx::M3::ContentToken::onSecondary]);
    style.Colors[ImGuiCol_SeparatorActive] =
        colors[ImGuiEx::M3::SurfaceToken::secondary].Pressed(colors[ImGuiEx::M3::ContentToken::onSecondary]);

    style.Colors[ImGuiCol_ResizeGrip]        = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_ResizeGripHovered] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Hovered(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );
    style.Colors[ImGuiCol_ResizeGripActive] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Pressed(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );

    style.Colors[ImGuiCol_InputTextCursor] = colors[ImGuiEx::M3::SurfaceToken::secondary];

    style.Colors[ImGuiCol_Tab] = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TabHovered] =
        colors[ImGuiEx::M3::SurfaceToken::surface].Hovered(colors[ImGuiEx::M3::ContentToken::onSurface]);
    style.Colors[ImGuiCol_TabSelected]         = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiEx::M3::SurfaceToken::primary];

    style.Colors[ImGuiCol_TabDimmed] = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TabDimmedSelected] =
        colors[ImGuiEx::M3::SurfaceToken::surface].Pressed(colors[ImGuiEx::M3::ContentToken::onSurface]);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_PlotLines] = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_PlotLinesHovered] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Hovered(colors[ImGuiEx::M3::ContentToken::onPrimary]);

    style.Colors[ImGuiCol_PlotHistogram] = colors[ImGuiEx::M3::SurfaceToken::tertiary];
    style.Colors[ImGuiCol_PlotHistogramHovered] =
        colors[ImGuiEx::M3::SurfaceToken::tertiary].Hovered(colors[ImGuiEx::M3::ContentToken::onTertiary]);

    style.Colors[ImGuiCol_TableHeaderBg]     = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_TableBorderStrong] = colors[ImGuiEx::M3::SurfaceToken::outline];
    style.Colors[ImGuiCol_TableBorderLight]  = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_TableRowBg]        = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TableRowBgAlt]     = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerLowest];

    // style.Colors[ImGuiCol_TextLink]     =colors.surface_container_low; // TODO: set this
    style.Colors[ImGuiCol_TextSelectedBg]   = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = colors[ImGuiEx::M3::ContentToken::onSurface];

    style.Colors[ImGuiCol_DragDropTarget]   = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_DragDropTargetBg] = colors[ImGuiEx::M3::SurfaceToken::surface];

    style.Colors[ImGuiCol_UnsavedMarker]         = colors[ImGuiEx::M3::ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavCursor]             = colors[ImGuiEx::M3::ContentToken::onSecondary];
    style.Colors[ImGuiCol_NavWindowingHighlight] = colors[ImGuiEx::M3::ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavWindowingDimBg]     = colors[ImGuiEx::M3::SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_ModalWindowDimBg]   = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}

}
}