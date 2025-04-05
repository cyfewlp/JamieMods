//
// Created by jamie on 2025/4/4.
//

#include "SosGuiMenu.h"

#include "SimpleIME/include/ImGuiThemeLoader.h"
#include "SosGui.h"

#include <common/log.h>

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

struct
{
    RE::GFxKey::Code gfxCode;
    ImGuiKey         imGuiKey;
} GFxCodeToImGuiKeyTable[] = {
    {RE::GFxKey::kReturn,      ImGuiKey_Enter         },
    {RE::GFxKey::kEscape,      ImGuiKey_Escape        },
    {RE::GFxKey::kLeft,        ImGuiKey_LeftArrow     },
    {RE::GFxKey::kUp,          ImGuiKey_UpArrow       },
    {RE::GFxKey::kRight,       ImGuiKey_RightArrow    },
    {RE::GFxKey::kDown,        ImGuiKey_DownArrow     },
    {RE::GFxKey::kSpace,       ImGuiKey_Space         },
    {RE::GFxKey::kBackspace,   ImGuiKey_Backspace     },
    {RE::GFxKey::kDelete,      ImGuiKey_Delete        },
    {RE::GFxKey::kInsert,      ImGuiKey_Insert        },
    {RE::GFxKey::kKP_Multiply, ImGuiKey_KeypadMultiply},
    {RE::GFxKey::kKP_Add,      ImGuiKey_KeypadAdd     },
    {RE::GFxKey::kKP_Enter,    ImGuiKey_KeypadEnter   },
    {RE::GFxKey::kKP_Subtract, ImGuiKey_KeypadSubtract},
    {RE::GFxKey::kKP_Decimal,  ImGuiKey_KeypadDecimal },
    {RE::GFxKey::kKP_Divide,   ImGuiKey_KeypadDivide  },
    {RE::GFxKey::kVoidSymbol,  ImGuiKey_None          }
};

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

    void SosGuiMenu::OnHide()
    {
        m_fShow = false;
        log_debug("SosGuiMenu::kHide");
        auto &io = ImGui::GetIO();
        io.ClearInputKeys();
        io.ClearInputCharacters();
    }

    RE::UI_MESSAGE_RESULTS SosGuiMenu::ProcessMessage(RE::UIMessage &a_message)
    {
        switch (a_message.type.get())
        {
            case RE::UI_MESSAGE_TYPE::kUpdate:
                break;
            case RE::UI_MESSAGE_TYPE::kUserEvent: {
                const auto &data = reinterpret_cast<RE::BSUIMessageData *>(a_message.data);
                log_debug("SosGuiMenu::kUserEvent {}", data->fixedStr.c_str());
                if (data->fixedStr == "Cancel")
                {
                    auto *messageQueue = RE::UIMessageQueue::GetSingleton();
                    messageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                }
                break;
            }
            case RE::UI_MESSAGE_TYPE::kShow:
                m_fShow = true;
                log_debug("SosGuiMenu::kShow");
                break;
            case RE::UI_MESSAGE_TYPE::kHide:
                OnHide();
                break;
            case RE::UI_MESSAGE_TYPE::kScaleformEvent: // never send because we use ImGui
            {
                auto *scaleformData = reinterpret_cast<RE::BSUIScaleformData *>(a_message.data);
                if (scaleformData != nullptr && scaleformData->scaleformEvent != nullptr)
                {
                    ProcessScaleformEvent(scaleformData);
                }
                break;
            }
            default:;
        }
        return IMenu::ProcessMessage(a_message);
    }

    void SosGuiMenu::ToggleShow()
    {
        m_fShow            = !m_fShow;
        auto  type         = m_fShow ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide;
        auto *messageQueue = RE::UIMessageQueue::GetSingleton();
        messageQueue->AddMessage(MENU_NAME, type, nullptr);
    }

        auto SosGuiMenu::Creator() -> IMenu *
        {
            using Flags = RE::UI_MENU_FLAGS;
            auto *menu  = new SosGuiMenu();
            menu->menuFlags.set(Flags::kPausesGame);
            menu->menuFlags.set(Flags::kUpdateUsesCursor, Flags::kUsesCursor);
            menu->menuFlags.set(Flags::kCustomRendering);
            menu->menuFlags.set(Flags::kTopmostRenderedMenu);
            menu->menuFlags.set(Flags::kUsesMenuContext);

            menu->inputContext.set(Context::kConsole);
            return menu;
        }

    void SosGuiMenu::ProcessScaleformEvent(RE::BSUIScaleformData *data)
    {
        const auto &fxEvent = data->scaleformEvent;
        const auto &type    = fxEvent->type.get();
        switch (type)
        {
            case RE::GFxEvent::EventType::kMouseDown:
                OnMouseEvent(fxEvent, true);
                break;
            case RE::GFxEvent::EventType::kMouseUp:
                OnMouseEvent(fxEvent, false);
                break;
            case RE::GFxEvent::EventType::kMouseWheel:
                OnMouseWheelEvent(fxEvent);
                break;
            case RE::GFxEvent::EventType::kKeyDown:
                OnKeyEvent(fxEvent, true);
                break;
            case RE::GFxEvent::EventType::kKeyUp:
                OnKeyEvent(fxEvent, false);
                break;
            case RE::GFxEvent::EventType::kCharEvent:
                OnCharEvent(fxEvent);
                break;
            default:
                break;
        }
    }

    void SosGuiMenu::OnMouseEvent(RE::GFxEvent *event, bool down)
    {
        auto *mouseEvent = reinterpret_cast<RE::GFxMouseEvent *>(event);
        ImGui::GetIO().AddMouseButtonEvent(static_cast<int>(mouseEvent->button), down);
    }

    void SosGuiMenu::OnMouseWheelEvent(RE::GFxEvent *event)
    {
        log_debug("ProcessScaleformEvent kMouseWheel");
        auto *mouseEvent = reinterpret_cast<RE::GFxMouseEvent *>(event);
        ImGui::GetIO().AddMouseWheelEvent(0, mouseEvent->scrollDelta);
    }

    void SosGuiMenu::OnKeyEvent(RE::GFxEvent *event, bool down)
    {
        auto keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);

        auto imguiKey = GFxKeyToImGuiKey(keyEvent->keyCode);
        ImGui::GetIO().AddKeyEvent(imguiKey, down);
    }

    void SosGuiMenu::OnCharEvent(RE::GFxEvent *event)
    {
        auto charEvent = reinterpret_cast<RE::GFxCharEvent *>(event);
        ImGui::GetIO().AddInputCharacter(charEvent->wcharCode);
    }

    auto SosGuiMenu::GFxKeyToImGuiKey(RE::GFxKey::Code keyCode) -> ImGuiKey
    {
        ImGuiKey imguiKey = ImGuiKey_None;

        if (keyCode >= RE::GFxKey::kA && keyCode <= RE::GFxKey::kZ)
        {
            imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kA + ImGuiKey_A);
        }
        else if (keyCode >= RE::GFxKey::kF1 && keyCode <= RE::GFxKey::kF15)
        {
            imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kF1 + ImGuiKey_F1);
        }
        else if (keyCode >= RE::GFxKey::kNum0 && keyCode <= RE::GFxKey::kNum9)
        {
            imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kNum0 + ImGuiKey_0);
        }
        else if (keyCode >= RE::GFxKey::kKP_0 && keyCode <= RE::GFxKey::kKP_9)
        {
            imguiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kKP_0 + ImGuiKey_Keypad0);
        }
        else
        {
            for (const auto &table : GFxCodeToImGuiKeyTable)
            {
                if (keyCode == table.gfxCode)
                {
                    imguiKey = table.imGuiKey;
                    break;
                }
            }
        }
        return imguiKey;
    }
}
