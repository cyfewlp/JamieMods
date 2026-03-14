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
#include "imguiex/imguiex_m3.h"
#include "log.h"
#include "path_utils.h"

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
    io.MouseDrawCursor          = true;
    io.ConfigNavMoveSetMousePos = false;

    static auto iniPath = (utils::GetInterfacePath() / SIMPLE_IME / "imgui.ini").generic_string();
    io.IniFilename      = iniPath.c_str();

    g_initialized = true;
    logger::info("ImGui initialized!");
}

auto AddPrimaryFont(const std::vector<std::string> &fontsPathList) -> ImFont *
{
    ImFont *imFont = AddFonts(fontsPathList);
    auto   &io     = ImGui::GetIO();
    if (imFont == nullptr)
    {
        ErrorNotifier::GetInstance().addError("Can't load fonts! Try fallback to the default fonts settings...", ErrorMsg::Level::warning);
        io.Fonts->Clear();
        auto defaultFonts = std::vector{std::string(Settings::DEFAULT_MAIN_FONT_PATH), std::string(Settings::DEFAULT_EMOJI_FONT_PATH)};
        imFont            = AddFonts(defaultFonts);
    }
    if (imFont == nullptr)
    {
        ErrorNotifier::GetInstance().addError("Can't load fonts! Fallback to ImGui embedded font...", ErrorMsg::Level::warning);
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

void InitializeM3(const std::filesystem::path &iconFontPath, const ImGuiEx::M3::SchemeConfig &schemeConfig)
{
    auto *iconFont = AddFont(iconFontPath.generic_string());
    if (iconFont == nullptr)
    {
        logger::error("Cannot find icon font from {}, fallback to default primary font!", iconFontPath.generic_string());
        iconFont = ImGui::GetFont();
    }

    ImGuiEx::M3::Context::CreateM3Styles(iconFont, {schemeConfig});
    ImGuiEx::M3::SetupDefaultImGuiStyles(ImGui::GetStyle());
}

void DestroyM3()
{
    ImGuiEx::M3::Context::DestroyM3Styles();
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

} // namespace Ime::UI
