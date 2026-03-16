//
// Created by jamie on 2026/1/1.
//

#include "menu/ImeMenu.h"

#include "ImeApp.h"
#include "RE/G/GFxEvent.h"
#include "RE/U/UI.h"
#include "RE/U/UIMessage.h"
#include "core/State.h"
#include "ime/ImeController.h"
#include "log.h"
#include "menu/MenuNames.h"
#include "menu/ToolWindowMenu.h"
#include "utils/Utils.h"

#include <unordered_map>

namespace Ime
{
namespace
{
auto ImeMenuCreator() -> RE::IMenu *
{
    using flags = RE::UI_MENU_FLAGS;
    auto *pMenu = new ImeMenu();
    pMenu->menuFlags.set(flags::kCustomRendering);
    pMenu->menuFlags.set(flags::kAlwaysOpen, flags::kAllowSaving);
    pMenu->depthPriority = 13;

    // pMenu->inputContext.set(Context::kCursor);
    // Priority 7: no render but no events
    // Priority 8: render but no events
    // Priority 9: render and have events

    return pMenu;
}

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

auto ImGui_ImplWin32_GetMouseSourceFromMessageExtraInfo() -> ImGuiMouseSource
{
    const LPARAM extra_info = GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700) return ImGuiMouseSource_Pen;         // NOLINT(*-magic-numbers)
    if ((extra_info & 0xFFFFFF80) == 0xFF515780) return ImGuiMouseSource_TouchScreen; // NOLINT(*-magic-numbers)
    return ImGuiMouseSource_Mouse;
}

// Send a fake KeyEvent to Console
// to avoid ignoring CharEvent due to the Console being held down by Ctrl.
auto SendFakeControlUpEvent() -> bool
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
            pEvent->type                          = static_cast<RE::GFxEvent::EventType>(GFxEventTypeEx::kImeKeyUp);
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

auto MapToImGuiKey(const RE::GFxKey::Code keyCode) -> ImGuiKey
{
    ImGuiKey imGuiKey = ImGuiKey_None;

    if (keyCode >= RE::GFxKey::kA && keyCode <= RE::GFxKey::kZ)
    {
        imGuiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kA + ImGuiKey_A);
    }
    else if (keyCode >= RE::GFxKey::kF1 && keyCode <= RE::GFxKey::kF15)
    {
        imGuiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kF1 + ImGuiKey_F1);
    }
    else if (keyCode >= RE::GFxKey::kNum0 && keyCode <= RE::GFxKey::kNum9)
    {
        imGuiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kNum0 + ImGuiKey_0);
    }
    else if (keyCode >= RE::GFxKey::kKP_0 && keyCode <= RE::GFxKey::kKP_9)
    {
        imGuiKey = static_cast<ImGuiKey>(keyCode - RE::GFxKey::kKP_0 + ImGuiKey_Keypad0);
    }
    else
    {
        auto foundIt = GFxCodeToImGuiKeyTable.find(keyCode);
        if (foundIt != GFxCodeToImGuiKeyTable.end())
        {
            imGuiKey = foundIt->second;
        }
    }
    return imGuiKey;
}

void SendKeyEventToImGui(const RE::GFxKeyEvent *keyEvent, const bool down)
{
    const auto imGuiKey = MapToImGuiKey(keyEvent->keyCode);
    ImGui::GetIO().AddKeyEvent(imGuiKey, down);
}

/**
 * We only handle @c CF_UNICODETEXT here. If the clipboard only provides @ CF_TEXT (ANSI), let the game handle paste
 * natively. This avoids incorrect Unicode injection and preserves vanilla behavior.
 */
bool Paste()
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
            Skyrim::SendUiString(std::wstring(textData));

            GlobalUnlock(handle);
        }
    }
    CloseClipboard();
    return true;
}

auto OnMouseEvent(RE::GFxEvent *event, const bool down) -> RE::UI_MESSAGE_RESULTS
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

auto OnMouseWheelEvent(RE::GFxEvent *event) -> RE::UI_MESSAGE_RESULTS
{
    const auto *mouseEvent = reinterpret_cast<RE::GFxMouseEvent *>(event);
    ImGui::GetIO().AddMouseWheelEvent(0, mouseEvent->scrollDelta);

    if (ToolWindowMenu::IsShowing())
    {
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}
} // namespace

void ImeMenu::PostDisplay()
{
    for (RE::GFxEvent *gfxEvent : m_imeCharEvents)
    {
        RE::GMemory::Free(gfxEvent);
    }
    m_imeCharEvents.clear();

#ifdef DEBUG
    const auto frameStart = std::chrono::high_resolution_clock::now();
    ImeApp::GetInstance().Draw();
    const auto frameEnd = std::chrono::high_resolution_clock::now();
    const auto frameMs  = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();

    static uint64_t                       s_frameCount = 0;
    static std::chrono::milliseconds::rep s_accumMs    = 0;
    static double                         avgMs        = 0;
    s_accumMs += frameMs;
    if (++s_frameCount % 60 == 0)
    {
        avgMs     = static_cast<double>(s_accumMs) / 60.0;
        s_accumMs = 0;
    }
    ImGui::Value("Avg frame(ms): ", static_cast<float>(avgMs));
#else
    ImeApp::GetInstance().Draw();
#endif
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
            if (scaleformData != nullptr && scaleformData->scaleformEvent != nullptr)
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
    switch (auto *fxEvent = data->scaleformEvent; fxEvent->type.get())
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
            const auto *charEvent = reinterpret_cast<GFxCharEvent *>(fxEvent);
            return OnCharEvent(charEvent);
        }
        case static_cast<RE::GFxEvent::EventType>(GFxEventTypeEx::kImeKeyUp): {
            auto *keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(fxEvent);
            m_imeCharEvents.push_back(keyEvent);
            keyEvent->type = RE::GFxEvent::EventType::kKeyUp;
            break;
        }
        case static_cast<RE::GFxEvent::EventType>(GFxEventTypeEx::kImeCharEvent): {
            // This cast is safe, but it's a bit hacky, since `kImeCharEvent` is not a real GFxEvent type, but we can use it to distinguish from
            // normal CharEvent.  THIS is key design that ensures our system functions properly and safety. The logic:
            // 1. intercept all the `kCharEvent` type events when IME activated. And call `Skyrim::SendUiString` when received `WM_CHAR` message.
            // 2. Pass all `GFxEventTypeEx::kImeCharEvent` event except for ToolWindowMenu is showing.
            // 3. Record these event and release it in `PostDisplay` to avoid memory leak.
            auto *charEvent = reinterpret_cast<GFxCharEvent *>(fxEvent);
            m_imeCharEvents.push_back(charEvent);
            if (ToolWindowMenu::IsShowing())
            {
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
    const auto *keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);
    SendKeyEventToImGui(keyEvent, down);
    if (keyEvent->keyCode == RE::GFxKey::kControl)
    {
        m_ctrlDown = down;
    }
    if (ToolWindowMenu::IsShowing())
    {
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

auto ImeMenu::OnCharEvent(const GFxCharEvent *charEvent) -> RE::UI_MESSAGE_RESULTS
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
    if (!state.ImeDisabled() && state.Has(Core::State::LANG_PROFILE_ACTIVATED))
    {
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

bool ImeMenu::IsPaste(const GFxCharEvent *charEvent) const
{
    return charEvent->wcharCode == 'v' && m_ctrlDown;
}

void ImeMenu::RegisterMenu()
{
    logger::info("Registering ToolWindowMenu...");
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        ui->Register(ImeMenuName, ImeMenuCreator);
    }
}
} // namespace Ime
