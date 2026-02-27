//
// Created by jamie on 2026/1/1.
//

#include "menu/ImeMenu.h"

#include "ImeApp.h"
#include "RE/G/GFxEvent.h"
#include "RE/U/UI.h"
#include "RE/U/UIMessage.h"
#include "Utils.h"
#include "core/State.h"
#include "ime/ImeController.h"
#include "log.h"
#include "menu/MenuNames.h"
#include "menu/ToolWindowMenu.h"

#include <unordered_map>

namespace Ime
{

namespace
{
std::unordered_map<RE::GFxKey::Code, ImGuiKey> GFxCodeToImGuiKeyTable = {
    {RE::GFxKey::kAlt,          ImGuiMod_Alt           },
    {RE::GFxKey::kControl,      ImGuiMod_Ctrl          },
    {RE::GFxKey::kShift,        ImGuiMod_Shift         },
    {RE::GFxKey::kCapsLock,     ImGuiKey_CapsLock      },
    // {RE::GFxKey::kTab,          ImGuiKey_Tab
    {RE::GFxKey::kHome,         ImGuiKey_Home          },
    {RE::GFxKey::kEnd,          ImGuiKey_End           },
    {RE::GFxKey::kPageUp,       ImGuiKey_PageUp        },
    {RE::GFxKey::kPageDown,     ImGuiKey_PageDown      },
    {RE::GFxKey::kComma,        ImGuiKey_Comma         },
    {RE::GFxKey::kPeriod,       ImGuiKey_Period        },
    {RE::GFxKey::kSlash,        ImGuiKey_Slash         },
    {RE::GFxKey::kBackslash,    ImGuiKey_Backslash     },
    {RE::GFxKey::kQuote,        ImGuiKey_Apostrophe    },
    {RE::GFxKey::kBracketLeft,  ImGuiKey_LeftBracket   },
    {RE::GFxKey::kBracketRight, ImGuiKey_RightBracket  },
    {RE::GFxKey::kReturn,       ImGuiKey_Enter         },
    {RE::GFxKey::kEqual,        ImGuiKey_Equal         },
    {RE::GFxKey::kMinus,        ImGuiKey_Minus         },
    {RE::GFxKey::kEscape,       ImGuiKey_Escape        },
    {RE::GFxKey::kLeft,         ImGuiKey_LeftArrow     },
    {RE::GFxKey::kUp,           ImGuiKey_UpArrow       },
    {RE::GFxKey::kRight,        ImGuiKey_RightArrow    },
    {RE::GFxKey::kDown,         ImGuiKey_DownArrow     },
    {RE::GFxKey::kSpace,        ImGuiKey_Space         },
    {RE::GFxKey::kBackspace,    ImGuiKey_Backspace     },
    {RE::GFxKey::kDelete,       ImGuiKey_Delete        },
    {RE::GFxKey::kInsert,       ImGuiKey_Insert        },
    {RE::GFxKey::kKP_Multiply,  ImGuiKey_KeypadMultiply},
    {RE::GFxKey::kKP_Add,       ImGuiKey_KeypadAdd     },
    {RE::GFxKey::kKP_Enter,     ImGuiKey_KeypadEnter   },
    {RE::GFxKey::kKP_Subtract,  ImGuiKey_KeypadSubtract},
    {RE::GFxKey::kKP_Decimal,   ImGuiKey_KeypadDecimal },
    {RE::GFxKey::kKP_Divide,    ImGuiKey_KeypadDivide  },
    {RE::GFxKey::kVoidSymbol,   ImGuiKey_None          }
};

ImGuiMouseSource ImGui_ImplWin32_GetMouseSourceFromMessageExtraInfo()
{
    const LPARAM extra_info = GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700) return ImGuiMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780) return ImGuiMouseSource_TouchScreen;
    return ImGuiMouseSource_Mouse;
}
} // namespace

void ImeMenu::PostDisplay()
{
    ImeApp::GetInstance().Draw();
}

auto ImeMenu::ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS
{
    if (!ImeApp::GetInstance().GetState().IsInitialized())
    {
        return RE::UI_MESSAGE_RESULTS::kPassOn;
    }
    switch (a_message.type.get())
    {
        case RE::UI_MESSAGE_TYPE::kShow:
            OnShow();
            break;
        case RE::UI_MESSAGE_TYPE::kHide:
            OnHide();
            break;
        case RE::UI_MESSAGE_TYPE::kScaleformEvent: {
            auto *scaleformData = reinterpret_cast<RE::BSUIScaleformData *>(a_message.data);
            if (scaleformData && scaleformData->scaleformEvent)
            {
                return ProcessScaleformEvent(scaleformData);
            }
        }
        break;
        default:;
    }
    return IMenu::ProcessMessage(a_message);
}

void ImeMenu::OnShow()
{
    logger::trace("ImeMenu: Show");
    m_fSShow = true;
}

void ImeMenu::OnHide()
{
    logger::trace("ImeMenu: Hide");
    m_fSShow = false;
}

auto ImeMenu::ProcessScaleformEvent(const RE::BSUIScaleformData *data) -> RE::UI_MESSAGE_RESULTS
{
    switch (const auto fxEvent = data->scaleformEvent; fxEvent->type.get())
    {
        case RE::GFxEvent::EventType::kKeyDown:
            return OnKeyEvent(fxEvent, true);
        case RE::GFxEvent::EventType::kKeyUp:
            return OnKeyEvent(fxEvent, false);
        case RE::GFxEvent::EventType::kMouseDown:
            return OnMouseEvent(fxEvent, true);
        case RE::GFxEvent::EventType::kMouseUp:
            return OnMouseEvent(fxEvent, false);
        case RE::GFxEvent::EventType::kMouseWheel:
            return OnMouseWheelEvent(fxEvent);
        case RE::GFxEvent::EventType::kCharEvent: {
            const auto charEvent = reinterpret_cast<RE::GFxCharEvent *>(fxEvent);
            return OnCharEvent(charEvent);
        }
        case static_cast<RE::GFxEvent::EventType>(GFxEventTypeEx::kImeCharEvent): {
            // This cast is safe but its a bit hacky, since kImeCharEvent is not a real GFxEvent type, but we can use it to distinguish from normal
            // CharEvent. The logic: intercept all the `RE::GFxEvent::EventType::kCharEvent` type events when IME activated. And call
            // `Utils::SendStringToGame` when recived `WM_CHAR` message. pass all `GFxEventTypeEx::kImeCharEvent` event except for ToolWindowMenu is
            // showing.
            // \todo Is this also worked when TSF disabled?
            if (ToolWindowMenu::IsShowing())
            {
                const auto charEvent = reinterpret_cast<RE::GFxCharEvent *>(fxEvent);
                ImGui::GetIO().AddInputCharacter(charEvent->wcharCode);
                return RE::UI_MESSAGE_RESULTS::kHandled;
            }
            fxEvent->type = RE::GFxEvent::EventType::kCharEvent;
            break;
        }
        default:
            break;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

auto ImeMenu::OnKeyEvent(RE::GFxEvent *event, const bool down) -> RE::UI_MESSAGE_RESULTS
{
    const auto keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);
    SendKeyEventToImGui(keyEvent, down);
    if (keyEvent->keyCode == RE::GFxKey::kControl)
    {
        ctrlDown = down;
    }
    if (ToolWindowMenu::IsShowing())
    {
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

void ImeMenu::SendKeyEventToImGui(const RE::GFxKeyEvent *keyEvent, const bool down)
{
    const auto imguiKey = MapToImGuiKey(keyEvent->keyCode);
    ImGui::GetIO().AddKeyEvent(imguiKey, down);
}

auto ImeMenu::MapToImGuiKey(const RE::GFxKey::Code keyCode) -> ImGuiKey
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
        auto foundIt = GFxCodeToImGuiKeyTable.find(keyCode);
        if (foundIt != GFxCodeToImGuiKeyTable.end())
        {
            imguiKey = foundIt->second;
        }
    }
    return imguiKey;
}

auto ImeMenu::OnMouseEvent(RE::GFxEvent *event, const bool down) -> RE::UI_MESSAGE_RESULTS
{
    const auto &mouseSource = ImGui_ImplWin32_GetMouseSourceFromMessageExtraInfo();
    const auto *mouseEvent  = reinterpret_cast<RE::GFxMouseEvent *>(event);
    auto       &io          = ImGui::GetIO();

    io.AddMouseSourceEvent(mouseSource);
    io.AddMouseButtonEvent(static_cast<int>(mouseEvent->button), down);

    if (ToolWindowMenu::IsShowing())
    {
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }

    if (Core::State::GetInstance().IsImeInputting())
    {
        if (!io.WantCaptureMouse)
        {
            // abort ime when click area is not ImeMenu when inputting;
            ImeApp::GetInstance().GetImeWnd().AbortIme();
        }
        return RE::UI_MESSAGE_RESULTS::kHandled; // avoid underlying menu losing input focus;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

auto ImeMenu::OnMouseWheelEvent(RE::GFxEvent *event) -> RE::UI_MESSAGE_RESULTS
{
    const auto *mouseEvent = reinterpret_cast<RE::GFxMouseEvent *>(event);
    ImGui::GetIO().AddMouseWheelEvent(0, mouseEvent->scrollDelta);

    if (ToolWindowMenu::IsShowing())
    {
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

// Send a fake KeyEvent to Console
// to avoid ignoring CharEvent due to the Console being held down by Ctrl.
bool SendFakeControlUpEvent()
{
    auto *pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
    auto *pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
    if (!pInterfaceStrings || !pFactoryManager)
    {
        return false;
    }
    if (const auto *pFactory = pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData))
    {
        if (auto *pScaleFormMessageData = pFactory->Create())
        {
            auto *pEvent                          = new RE::GFxKeyEvent();
            pEvent->type                          = RE::GFxEvent::EventType::kKeyUp;
            pEvent->keyCode                       = RE::GFxKey::kControl;
            pScaleFormMessageData->scaleformEvent = pEvent;

            RE::UIMessageQueue::GetSingleton()->AddMessage(
                RE::InterfaceStrings::GetSingleton()->console, RE::UI_MESSAGE_TYPE::kScaleformEvent, pScaleFormMessageData
            );
            return true;
        }
    }

    return false;
}

auto ImeMenu::OnCharEvent(const RE::GFxCharEvent *charEvent) -> RE::UI_MESSAGE_RESULTS
{
    if (ToolWindowMenu::IsShowing())
    {
        ImGui::GetIO().AddInputCharacter(charEvent->wcharCode);
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }

    if (!ImeController::GetInstance()->IsModEnabled())
    {
        return RE::UI_MESSAGE_RESULTS::kPassOn;
    }

    if (IsPaste(charEvent))
    {
        return SendFakeControlUpEvent() && Paste() ? RE::UI_MESSAGE_RESULTS::kHandled : RE::UI_MESSAGE_RESULTS::kPassOn;
    }

    const auto &state = Core::State::GetInstance();
    if (state.Has(Core::State::LANG_PROFILE_ACTIVATED))
    {
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

bool ImeMenu::IsPaste(const RE::GFxCharEvent *charEvent) const
{
    return charEvent->wcharCode == 'v' && ctrlDown;
}

/**
 * We only handle @c CF_UNICODETEXT here. If the clipboard only provides @ CF_TEXT (ANSI), let the game handle paste
 * natively. This avoids incorrect Unicode injection and preserves vanilla behavior.
 */
bool ImeMenu::Paste()
{
    if (OpenClipboard(nullptr) == FALSE)
    {
        return false;
    }

    if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE)
    {
        CloseClipboard();
        return false;
    }

    if (const HANDLE handle = GetClipboardData(CF_UNICODETEXT); handle != nullptr)
    {
        if (auto *const textData = static_cast<LPTSTR>(GlobalLock(handle)); textData != nullptr)
        {
            Utils::SendStringToGame(std::wstring(textData));

            GlobalUnlock(handle);
        }
    }
    CloseClipboard();
    return true;
}

void ImeMenu::RegisterMenu()
{
    logger::info("Registering ToolWindowMenu...");
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        ui->Register(ImeMenuName, Creator);
    }
}

auto ImeMenu::Creator() -> IMenu *
{
    using flags = RE::UI_MENU_FLAGS;
    auto *pMenu = new ImeMenu();
    pMenu->menuFlags.set(flags::kCustomRendering);
    pMenu->menuFlags.set(flags::kAlwaysOpen);
    pMenu->depthPriority = 13;

    // pMenu->inputContext.set(Context::kCursor);
    // Priority 7: no render but no events
    // Priority 8: render but no events
    // Priority 9: render and have events

    return pMenu;
}
} // namespace Ime
