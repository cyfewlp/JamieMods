//
// Created by jamie on 2026/2/13.
//

#pragma once

#include "imgui_te_context.h"
#include "imgui_te_engine.h"

#include "gtest/gtest.h"
#include <atlcomcli.h>
#include <d3d11.h>
#include <memory>
#include <windows.h>

namespace ImGuiEx::M3
{
class M3Styles;
}

namespace ImGuiEx::M3::detail
{
class M3TestFixture : public testing::Test
{
public:
    static constexpr auto WND_CLASS_NAME = L"M3Test";

    struct DX11RenderState
    {
        CComPtr<ID3D11Device>           d3dDevice            = nullptr;
        CComPtr<ID3D11DeviceContext>    d3DeviceContext      = nullptr;
        CComPtr<IDXGISwapChain>         swapChain            = nullptr;
        CComPtr<ID3D11RenderTargetView> mainRenderTargetView = nullptr;
    };

private:
    ImGuiTestEngine          *m_testEngine = nullptr;
    DX11RenderState           m_dx11RenderState;
    HWND                      m_hwnd     = nullptr;
    bool                      m_headless = false;
    std::unique_ptr<M3Styles> m_pM3Styles;

public:
    ~M3TestFixture() override = default;

    void Initialize(bool headless);
    void Shutdown();

    void RegisterTest(auto &&RegisterFunc) { RegisterFunc(m_testEngine, *m_pM3Styles); }

    [[nodiscard]] auto IsHeadless() const -> bool { return m_headless; }

    void RunLoop();

private:
    static auto WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
};

} // namespace ImGuiEx::M3::detail
