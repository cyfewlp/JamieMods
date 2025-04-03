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
    auto SosGui::ToggleShow() -> void
    {
        m_fShow = !m_fShow;

        if (m_fShow)
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ListActors);
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

            RenderLocationBasedAutoswitch();

            ImGui::Button("Export to Json");
            ImGui::SameLine();
            ImGui::Button("Import from Json");
        }
        ImGui::End();
    }

    void SosGui::RenderCharactersConfig()
    {
        ImGui::SeparatorText("Characters");
        static bool fShowNearNpcLis = false;
        if (ImGui::Checkbox("$SkyOutSys_Text_AddActorSelection", &fShowNearNpcLis))
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
                        }
                        if (selectedIdx == idx)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                        idx++;
                    }
                    ImGui::EndCombo();
                }
                ImGui::SameLine();
                if (ImGui::Button("Add NPC"))
                {
                    PapyrusEvent::GetInstance().CallAddActor(nearActors.at(selectedIdx));
                }
            }
        }
        RenderCharactersList();
    }

    void SosGui::RenderCharactersList()
    {
        const auto &actors = SosUiData::GetInstance().GetActors();
        if (ImGui::Button("Refresh"))
        {
            PapyrusEvent::GetInstance().CallNoArgs(SosFunctionNames::ListActors);
        }
        if (ImGui::BeginTable("CharactersList", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("Actor");
            ImGui::TableSetupColumn("Remove");
            ImGui::TableHeadersRow();
            for (const auto &actor : actors)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", actor->GetName());

                ImGui::TableNextColumn();
                if (ImGui::Button(std::format("Remove##{}", actor->GetName()).c_str()))
                {
                    PapyrusEvent::GetInstance().CallRemoveActor(actor);
                }
            }
            ImGui::EndTable();
        }
    }

    void SosGui::RenderLocationBasedAutoswitch()
    {
        static bool enableLocationBasedAutoswitch = false;
        ImGui::Checkbox("Location-Based Autoswitch", &enableLocationBasedAutoswitch);

    }
}
