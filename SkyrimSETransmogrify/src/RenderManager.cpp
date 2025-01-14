// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "RenderManager.h"

#include <tchar.h>

#include "Hooks.h"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "utils.hpp"
#include <unordered_set>

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define DEFAULT_CAND_NUM_PER_PAGE 5

static bool imeChoseCand = false;
extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // Use ImGui::GetCurrentContext()

void
RenderManager::UpdateCandStrings(HWND hWnd)
{
    HIMC hIMC = ImmGetContext(hWnd);
    if (!hIMC)
        return;
    DWORD dwBufLen = ImmGetCandidateList(hIMC, 0, nullptr, 0); // Get size first
    if (dwBufLen == 0) {
        ImmReleaseContext(hWnd, hIMC);
        return;
    }
    std::vector<BYTE> buffer(dwBufLen);
    LPCANDIDATELIST lpCandList = reinterpret_cast<LPCANDIDATELIST>(buffer.data());
    if (!lpCandList) {
        LOG(err, "Alloc for LPCANDIDATELIST failed!");
        return;
    }
    ImmGetCandidateList(hIMC, 0, lpCandList, dwBufLen);
    if (lpCandList->dwStyle == IME_CAND_CODE)
        return;
    uiState.candStrList.clear();
    uiState.selectedCand = lpCandList->dwSelection;
    utils::GetCandStrings(hWnd, lpCandList, &uiState.candStrList);
    ImmReleaseContext(hWnd, hIMC);
}

LRESULT
RenderManager::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto& io = ImGui::GetIO();
    switch (uMsg) {
        case WM_IME_KEYDOWN:
        case WM_KEYDOWN:
            if (imeChoseCand && wParam == VK_PROCESSKEY)
                wParam = ImmGetVirtualKey(hWnd);
            switch (wParam) {
                case VK_UP:
                case VK_DOWN:
                    LOG(debug, "Intercept up, down on ime.");
                    return S_OK;
            }
            break;
        case WM_INPUTLANGCHANGE: {
            UpdateKeyboardCodePage(io);
            break;
        }
        case WM_IME_NOTIFY: {
            switch (wParam) {
                case IMN_OPENCANDIDATE:
                    imeChoseCand = true;
                    UpdateCandStrings(hWnd);
                    break;
                case IMN_SETCANDIDATEPOS:
                case IMN_CHANGECANDIDATE: {
                    UpdateCandStrings(hWnd);
                    break;
                }
                case IMN_CLOSECANDIDATE:
                    imeChoseCand = false;
                    break;
            }
            return S_OK;
        }
        case WM_IME_ENDCOMPOSITION: {
            uiState.candStrList.clear();
            break;
        }
        case WM_IME_CHAR: {
            char mbstr[3]{ '\0' };
            BYTE hiByte = HIBYTE(wParam);
            BYTE loByte = LOBYTE(wParam);
            if (hiByte == 0) {
                mbstr[0] = loByte;
            } else {
                mbstr[0] = hiByte;
                mbstr[1] = loByte;
            }
            wchar_t wstr[2];
            int num = MultiByteToWideChar(uiState.KeyboardCodePage, MB_PRECOMPOSED, mbstr, -1, wstr, _countof(wstr));
            io.AddInputCharacter(wstr[0]);
            io.AddInputCharacter(wstr[1]);
            utils::NormalizeTest(wstr);
            LOG(info, "ime char: {}", utils::WideStrToStrUTF8(wstr));
            return S_OK;
        }
        case WM_IME_SETCONTEXT:
            return DefWindowProcA(hWnd, WM_IME_SETCONTEXT, wParam, NULL);
        default:
            break;
    }
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    return RealWndProc(hWnd, uMsg, wParam, lParam);
}

LRESULT WndProc1(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void
RenderManager::Init(Config* a_config)
{
    RealD3dInitFunc = Hooks::D3DInitHook::Install(&D3DInit);
    RealD3dPresentFunc = Hooks::D3DPresentHook::Install(&D3DPresent);
    config = a_config;
    SKSE::GetTaskInterface()->AddTask([] {
        Transmogrify::GetAllArmors(&uiState.modToArmors);
        for (auto& pair : uiState.modToArmors) {
            uiState.mods.push_back(pair.first);
        }
    });
}

void
RenderManager::D3DInit()
{
    RealD3dInitFunc();

    LOG(info, "D3DInit Hooked!");
    auto render_manager = RE::BSGraphics::Renderer::GetSingleton();
    if (!render_manager) {
        LOG(err, "Cannot find render manager. Initialization failed!");
        return;
    }

    auto render_data = render_manager->data;

    LOG(info, "Getting SwapChain...");
    auto pSwapChain = render_data.renderWindows->swapChain;
    if (!pSwapChain) {
        LOG(err, "Cannot find SwapChain. Initialization failed!");
        return;
    }

    LOG(info, "Getting SwapChain desc...");
    DXGI_SWAP_CHAIN_DESC sd{};
    if (pSwapChain->GetDesc(std::addressof(sd)) < 0) {
        LOG(err, "IDXGISwapChain::GetDesc failed.");
        return;
    }

    auto device = render_data.forwarder;
    auto context = render_data.context;

    LOG(info, "Initializing ImGui...");
    ImGui::CreateContext();
    if (!ImGui_ImplWin32_Init(sd.OutputWindow)) {
        LOG(err, "ImGui initialization failed (Win32)");
        return;
    }
    if (!ImGui_ImplDX11_Init(device, context)) {
        LOG(err, "ImGui initialization failed (DX11)");
        return;
    }

    ConfigImGui(sd.OutputWindow);
    initialized.store(true);

    RealWndProc = reinterpret_cast<WNDPROC>(
      SetWindowLongPtrA(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(RenderManager::WndProc)));
    if (!RealWndProc) {
        LOG(err, "SetWindowLongPtrA failed!");
    }
}

static HWND testImeWnd;
void
createPopupWindow(HWND parent)
{
    WNDCLASSEXW wc = { sizeof(wc),
                       CS_CLASSDC,
                       WndProc1,
                       0L,
                       0L,
                       (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE),
                       nullptr,
                       nullptr,
                       nullptr,
                       nullptr,
                       L"Test popup window",
                       nullptr };
    ::RegisterClassExW(&wc);
    testImeWnd = ::CreateWindowW(wc.lpszClassName,
                                 L"Test IME Window",
                                 WS_BORDER | WS_POPUP | WS_DISABLED,
                                 100,
                                 100,
                                 200,
                                 200,
                                 parent,
                                 nullptr,
                                 wc.hInstance,
                                 nullptr);
    ::ShowWindow(testImeWnd, SW_SHOWDEFAULT);
    LOG(debug, "Test popupwindow created");
}

void
RenderManager::ConfigImGui(HWND hwnd)
{
    RECT rect = { 0, 0, 0, 0 };
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    GetClientRect(hwnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
    io.MouseDrawCursor = true;
    io.ConfigNavMoveSetMousePos = true;

    io.Fonts->AddFontFromFileTTF(
      config->eastAsiaFontFile.c_str(), config->fontSize, nullptr, io.Fonts->GetGlyphRangesChineseFull());

    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static const ImWchar icons_ranges[] = { 0x1F000, 0x1FFFF, 0 }; // Will not be copied
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", config->fontSize, &cfg, icons_ranges);
    io.Fonts->Build();

    UpdateKeyboardCodePage(io);
    ImGui::GetMainViewport()->PlatformHandleRaw = (void*)hwnd;

    LOG(info, "ImGui initialized!");
    createPopupWindow(hwnd);
}

void
RenderManager::D3DPresent(std::uint32_t a_p1)
{
    RealD3dPresentFunc(a_p1);
    if (!initialized.load() || !RenderManager::showWindow) {
        return;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    render();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void
RenderManager::render()
{
    static char buf[64] = "";
    ImGui::Begin("SkyrimTransmogrify");
    ImGui::Checkbox("Show Mods", &RenderManager::uiState.showArmors);
    ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf));

    if (!uiState.candStrList.empty()) {
        ImGui::SetKeyboardFocusHere(-1);
        for (size_t i = 0; i < uiState.candStrList.size(); i++) {
            bool selected = uiState.selectedCand == i;
            std::string iCandStr = utils::WideStrToStrUTF8(uiState.candStrList[i]);
            std::string fmt = std::format("{}. {}", i + 1, iCandStr);
            if (selected) {
                ImGui::TextColored((ImVec4)ImColor(IM_COL32(109, 182, 218, 255)), fmt.c_str());
            } else {
                ImGui::Text(fmt.c_str());
            }
        }
    }

    if (RenderManager::uiState.showArmors) {
        RenderArmors();
    }

    if (ImGui::Button("Access player inventory")) {
        Transmogrify::AccessPlayerInventory();
    }
    ImGui::End();
}

void
RenderManager::RenderArmors()
{
    if (ImGui::BeginCombo("Mods", uiState.mods[uiState.selectedMod].c_str())) {
        for (int n = 0; n < uiState.mods.size(); n++) {
            const bool is_selected = (uiState.selectedMod == n);
            if (ImGui::Selectable(uiState.mods[n].c_str(), is_selected))
                uiState.selectedMod = n;

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void
RenderManager::SwitchShowWindow()
{
    showWindow = !showWindow;
}

void
RenderManager::cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool
RenderManager::IsShowWindow()
{
    return showWindow;
}

void
RenderManager::UpdateKeyboardCodePage(ImGuiIO& io)
{
    // Retrieve keyboard code page, required for handling of non-Unicode Windows.
    HKL keyboard_layout = ::GetKeyboardLayout(0);
    LCID keyboard_lcid = MAKELCID(HIWORD(keyboard_layout), SORT_DEFAULT);
    if (::GetLocaleInfoA(keyboard_lcid,
                         (LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE),
                         (LPSTR)&uiState.KeyboardCodePage,
                         sizeof(uiState.KeyboardCodePage)) == 0)
        uiState.KeyboardCodePage = CP_ACP; // Fallback to default ANSI code page when fails.
}
