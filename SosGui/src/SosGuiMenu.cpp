//
// Created by jamie on 2025/4/4.
//

#include "SosGuiMenu.h"

#include "SimpleIME/include/ImGuiThemeLoader.h"
#include "SosGui.h"

#include <common/log.h>

extern ImGuiKey ImGui_ImplWin32_KeyEventToImGuiKey(WPARAM wParam, LPARAM lParam);

namespace RE
{
    class GFxCharEvent : public RE::GFxEvent
    {
    public:
        GFxCharEvent() = default;

        explicit GFxCharEvent(UINT32 a_wcharCode, UINT8 a_keyboardIndex = 0)
            : GFxEvent(EventType::kCharEvent), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
        {
        }

        // @members
        std::uint32_t wcharCode{};     // 04
        std::uint32_t keyboardIndex{}; // 08
    };

    static_assert(sizeof(GFxCharEvent) == 0x0C);
}

namespace LIBC_NAMESPACE_DECL
{
    void SosGuiMenu::RegisterMenu()
    {
        if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
        {
            ui->Register(MENU_NAME, Creator);
        }
    }

    void SosGuiMenu::PostDisplay()
    {
        SosGui::GetInstance().Render();
    }

    RE::UI_MESSAGE_RESULTS SosGuiMenu::ProcessMessage(RE::UIMessage &a_message)
    {
        log_debug("SosGuiMenu::ProcessMessage {}", a_message.type.underlying());
        switch (a_message.type.get())
        {
            case RE::UI_MESSAGE_TYPE::kShow:
                break;
            case RE::UI_MESSAGE_TYPE::kHide:

                break;
            case RE::UI_MESSAGE_TYPE::kScaleformEvent: // never send because we use ImGui
                break;
            default:;
        }
        return IMenu::ProcessMessage(a_message);
    }

    auto SosGuiMenu::Creator() -> IMenu *
    {
        auto *menu = new SosGuiMenu();
        menu->menuFlags.set(RE::UI_MENU_FLAGS::kPausesGame);
        menu->menuFlags.set(RE::UI_MENU_FLAGS::kUpdateUsesCursor, RE::UI_MENU_FLAGS::kUsesCursor);
        menu->menuFlags.set(RE::UI_MENU_FLAGS::kCustomRendering);
        menu->menuFlags.set(RE::UI_MENU_FLAGS::kTopmostRenderedMenu);
        return menu;
    }
}
