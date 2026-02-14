//
// Created by jamie on 2026/1/16.
//
#include "ui/imgui_system.h"

#include "ime/ImeController.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imguiex/ErrorNotifier.h"
#include "imguiex/Material3.h"
#include "log.h"

#include <RE/C/ControlMap.h>
#include <RE/C/CursorMenu.h>
#include <RE/M/MenuCursor.h>
#include <RE/U/UI.h>
#include <RE/U/UserEvents.h>
#include <d3d11.h>
#include <windef.h>

namespace Ime::UI
{

namespace
{
bool g_initialized = false;

auto AddFonts(const std::vector<std::string> &fontPaths) -> ImFont *
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
void UpdateCursorPos()
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

/**
 * @brief Notifies SkyrimSE to direct character events to ImeMenu.
 * We call @c ControlMap::AllowTextInput and @c ImeController::EnableIme to manage focus correctly.
 * This avoids the IME remaining enabled if the underlying menu
 * also has a text entry field.
 */
void EnableTextInputIfNeed()
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

} // namespace

void Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context)
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

    g_initialized = true;
    logger::info("ImGui initialized!");
}

auto AddPrimaryFont(const std::vector<std::string> &fontsPathList) -> ImFont *
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

auto AddFont(const std::string &filePath) -> ImFont *
{
    return ImGui::GetIO().Fonts->AddFontFromFileTTF(filePath.c_str());
}

void NewFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    UpdateCursorPos();
    ImGui::NewFrame();

    EnableTextInputIfNeed();
}

void EndFrame() {}

void Render()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Shutdown()
{
    if (g_initialized)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
    g_initialized = false;
}

void ApplyM3Theme(const ImGuiEx::M3::M3Styles &m3Styles)
{
    using ColorRole   = ImGuiEx::M3::ColorRole;
    ImGuiStyle &style = ImGui::GetStyle();

    const auto &colors = m3Styles.Colors();

    style.Colors[ImGuiCol_Text]             = colors[ColorRole::onSurface];
    style.Colors[ImGuiCol_TitleBg]          = colors[ColorRole::surfaceContainer];
    style.Colors[ImGuiCol_TitleBgActive]    = colors[ColorRole::surface];
    style.Colors[ImGuiCol_TitleBgCollapsed] = colors[ColorRole::surfaceContainer];

    style.Colors[ImGuiCol_WindowBg] = colors[ColorRole::surface];
    style.Colors[ImGuiCol_ChildBg]  = colors[ColorRole::surface];
    style.Colors[ImGuiCol_PopupBg]  = colors[ColorRole::primaryContainer];

    style.Colors[ImGuiCol_FrameBg] = colors[ColorRole::secondaryContainer];
    style.Colors[ImGuiCol_FrameBgActive] =
        colors.Pressed(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);
    style.Colors[ImGuiCol_FrameBgHovered] =
        colors.Hovered(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);
    style.Colors[ImGuiCol_Border]       = colors[ColorRole::outlineVariant];
    style.Colors[ImGuiCol_BorderShadow] = colors[ColorRole::outlineVariant];

    style.Colors[ImGuiCol_SliderGrab]       = colors[ColorRole::primary];
    style.Colors[ImGuiCol_SliderGrabActive] = colors.Pressed(ColorRole::primary, ColorRole::onPrimary);

    style.Colors[ImGuiCol_Button]        = colors[ColorRole::primary];
    style.Colors[ImGuiCol_ButtonHovered] = colors.Hovered(ColorRole::primary, ColorRole::onPrimary);
    style.Colors[ImGuiCol_ButtonActive]  = colors.Pressed(ColorRole::primary, ColorRole::onPrimary);

    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = colors[ColorRole::outline];
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = colors[ColorRole::outlineVariant];
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = colors[ColorRole::primary];

    style.Colors[ImGuiCol_MenuBarBg] = colors[ColorRole::surfaceContainerHigh];

    style.Colors[ImGuiCol_Header]        = colors[ColorRole::surfaceContainerHigh];
    style.Colors[ImGuiCol_HeaderHovered] = colors.Hovered(ColorRole::surfaceContainerHigh, ColorRole::onSurface);
    style.Colors[ImGuiCol_HeaderActive]  = colors.Pressed(ColorRole::surfaceContainerHigh, ColorRole::onSurface);

    style.Colors[ImGuiCol_Separator]        = colors[ColorRole::secondary];
    style.Colors[ImGuiCol_SeparatorHovered] = colors.Hovered(ColorRole::secondary, ColorRole::onSecondary);
    style.Colors[ImGuiCol_SeparatorActive]  = colors.Pressed(ColorRole::secondary, ColorRole::onSecondary);

    style.Colors[ImGuiCol_ResizeGrip] = colors[ColorRole::secondaryContainer];
    style.Colors[ImGuiCol_ResizeGripHovered] =
        colors.Hovered(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);
    style.Colors[ImGuiCol_ResizeGripActive] =
        colors.Pressed(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);

    style.Colors[ImGuiCol_InputTextCursor] = colors[ColorRole::secondary];

    style.Colors[ImGuiCol_Tab]                 = colors[ColorRole::surface];
    style.Colors[ImGuiCol_TabHovered]          = colors.Hovered(ColorRole::surface, ColorRole::onSurface);
    style.Colors[ImGuiCol_TabSelected]         = colors[ColorRole::surface];
    style.Colors[ImGuiCol_TabSelectedOverline] = colors[ColorRole::primary];

    style.Colors[ImGuiCol_TabDimmed]                 = colors[ColorRole::surface];
    style.Colors[ImGuiCol_TabDimmedSelected]         = colors.Pressed(ColorRole::surface, ColorRole::onSurface);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = colors[ColorRole::outlineVariant];

    style.Colors[ImGuiCol_PlotLines]        = colors[ColorRole::primary];
    style.Colors[ImGuiCol_PlotLinesHovered] = colors.Hovered(ColorRole::primary, ColorRole::onPrimary);

    style.Colors[ImGuiCol_PlotHistogram]        = colors[ColorRole::tertiary];
    style.Colors[ImGuiCol_PlotHistogramHovered] = colors.Hovered(ColorRole::tertiary, ColorRole::onTertiary);

    style.Colors[ImGuiCol_TableHeaderBg]     = colors[ColorRole::surfaceContainerHigh];
    style.Colors[ImGuiCol_TableBorderStrong] = colors[ColorRole::outline];
    style.Colors[ImGuiCol_TableBorderLight]  = colors[ColorRole::outlineVariant];
    style.Colors[ImGuiCol_TableRowBg]        = colors[ColorRole::surface];
    style.Colors[ImGuiCol_TableRowBgAlt]     = colors[ColorRole::surfaceContainerLowest];

    // style.Colors[ImGuiCol_TextLink]     =colors.surface_container_low; // TODO: set this
    style.Colors[ImGuiCol_TextSelectedBg]   = colors[ColorRole::primary];
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = colors[ColorRole::onSurface];

    style.Colors[ImGuiCol_DragDropTarget]   = colors[ColorRole::primary];
    style.Colors[ImGuiCol_DragDropTargetBg] = colors[ColorRole::surface];

    style.Colors[ImGuiCol_UnsavedMarker]         = colors[ColorRole::onPrimary];
    style.Colors[ImGuiCol_NavCursor]             = colors[ColorRole::onSecondary];
    style.Colors[ImGuiCol_NavWindowingHighlight] = colors[ColorRole::onPrimary];
    style.Colors[ImGuiCol_NavWindowingDimBg]     = colors[ColorRole::surfaceContainer];

    style.Colors[ImGuiCol_ModalWindowDimBg]   = colors[ColorRole::surface];
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}

} // namespace Ime::UI
