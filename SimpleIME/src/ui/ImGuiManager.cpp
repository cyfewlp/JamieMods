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

namespace Ime
{

void ImGuiManager::Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context, Settings &settings)
{
    if (g_initialized)
    {
        logger::warn("ImGui already initialized!");
        return;
    }
    g_initialized = false;

    logger::info("Initializing ImGui...");
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(device, context);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigNavMoveSetMousePos = false;

    settings.state.dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);

    g_initialized = true;
    logger::info("ImGui initialized!");
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
    const bool  cWantTextInput = ImGui::GetIO().WantTextInput;
    const auto *imeManager     = ImeController::GetInstance();

    auto *controlMap = RE::ControlMap::GetSingleton();
    if (!fWantTextInput && cWantTextInput)
    {
        controlMap->AllowTextInput(true);
        imeManager->EnableIme(true);
        controlMap->StoreControls();
        controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kMenu, false, false);
    }
    else if (fWantTextInput && !cWantTextInput)
    {
        controlMap->AllowTextInput(false);
        imeManager->EnableIme(false);
        controlMap->ToggleControls(RE::UserEvents::USER_EVENT_FLAG::kMenu, true, false);
        controlMap->LoadStoredControls();
    }
    fWantTextInput = cWantTextInput;
}

void ImGuiManager::EndFrame() {}

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

void ImGuiManager::ApplyM3Theme(const ImGuiEx::M3::M3Styles &m3Styles)
{
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;
    ImGuiStyle &style  = ImGui::GetStyle();

    const auto &colors = m3Styles.Colors();

    style.Colors[ImGuiCol_Text]             = colors[ContentToken::onSurface];
    style.Colors[ImGuiCol_TitleBg]          = colors[SurfaceToken::surfaceContainer];
    style.Colors[ImGuiCol_TitleBgActive]    = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TitleBgCollapsed] = colors[SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_WindowBg] = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_ChildBg]  = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_PopupBg]  = colors[SurfaceToken::primaryContainer];

    style.Colors[ImGuiCol_FrameBg] = colors[SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_FrameBgActive] =
        colors.Pressed(SurfaceToken::secondaryContainer, ContentToken::onSecondaryContainer);
    style.Colors[ImGuiCol_FrameBgHovered] =
        colors.Hovered(SurfaceToken::secondaryContainer, ContentToken::onSecondaryContainer);
    style.Colors[ImGuiCol_Border]       = colors[SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_BorderShadow] = colors[SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_SliderGrab]       = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_SliderGrabActive] = colors.Pressed(SurfaceToken::primary, ContentToken::onPrimary);

    style.Colors[ImGuiCol_Button]        = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_ButtonHovered] = colors.Hovered(SurfaceToken::primary, ContentToken::onPrimary);
    style.Colors[ImGuiCol_ButtonActive]  = colors.Pressed(SurfaceToken::primary, ContentToken::onPrimary);

    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = colors[SurfaceToken::outline];
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = colors[SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = colors[SurfaceToken::primary];

    style.Colors[ImGuiCol_MenuBarBg] = colors[SurfaceToken::surfaceContainerHigh];

    style.Colors[ImGuiCol_Header]        = colors[SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_HeaderHovered] = colors.Hovered(SurfaceToken::surfaceContainerHigh, ContentToken::onSurface);
    style.Colors[ImGuiCol_HeaderActive]  = colors.Pressed(SurfaceToken::surfaceContainerHigh, ContentToken::onSurface);

    style.Colors[ImGuiCol_Separator]        = colors[SurfaceToken::secondary];
    style.Colors[ImGuiCol_SeparatorHovered] = colors.Hovered(SurfaceToken::secondary, ContentToken::onSecondary);
    style.Colors[ImGuiCol_SeparatorActive]  = colors.Pressed(SurfaceToken::secondary, ContentToken::onSecondary);

    style.Colors[ImGuiCol_ResizeGrip] = colors[SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_ResizeGripHovered] =
        colors.Hovered(SurfaceToken::secondaryContainer, ContentToken::onSecondaryContainer);
    style.Colors[ImGuiCol_ResizeGripActive] =
        colors.Pressed(SurfaceToken::secondaryContainer, ContentToken::onSecondaryContainer);

    style.Colors[ImGuiCol_InputTextCursor] = colors[SurfaceToken::secondary];

    style.Colors[ImGuiCol_Tab]                 = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TabHovered]          = colors.Hovered(SurfaceToken::surface, ContentToken::onSurface);
    style.Colors[ImGuiCol_TabSelected]         = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TabSelectedOverline] = colors[SurfaceToken::primary];

    style.Colors[ImGuiCol_TabDimmed]                 = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TabDimmedSelected]         = colors.Pressed(SurfaceToken::surface, ContentToken::onSurface);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = colors[SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_PlotLines]        = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_PlotLinesHovered] = colors.Hovered(SurfaceToken::primary, ContentToken::onPrimary);

    style.Colors[ImGuiCol_PlotHistogram]        = colors[SurfaceToken::tertiary];
    style.Colors[ImGuiCol_PlotHistogramHovered] = colors.Hovered(SurfaceToken::tertiary, ContentToken::onTertiary);

    style.Colors[ImGuiCol_TableHeaderBg]     = colors[SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_TableBorderStrong] = colors[SurfaceToken::outline];
    style.Colors[ImGuiCol_TableBorderLight]  = colors[SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_TableRowBg]        = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TableRowBgAlt]     = colors[SurfaceToken::surfaceContainerLowest];

    // style.Colors[ImGuiCol_TextLink]     =colors.surface_container_low; // TODO: set this
    style.Colors[ImGuiCol_TextSelectedBg]   = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = colors[ContentToken::onSurface];

    style.Colors[ImGuiCol_DragDropTarget]   = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_DragDropTargetBg] = colors[SurfaceToken::surface];

    style.Colors[ImGuiCol_UnsavedMarker]         = colors[ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavCursor]             = colors[ContentToken::onSecondary];
    style.Colors[ImGuiCol_NavWindowingHighlight] = colors[ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavWindowingDimBg]     = colors[SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_ModalWindowDimBg]   = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}

} // namespace Ime
