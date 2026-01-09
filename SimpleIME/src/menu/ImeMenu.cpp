//
// Created by jamie on 2026/1/1.
//

#include "menu/ImeMenu.h"

#include "ImeApp.h"
#include "RE/G/GFxEvent.h"
#include "RE/U/UI.h"
#include "RE/U/UIMessage.h"
#include "Utils.h"
#include "common/log.h"
#include "core/State.h"
#include "menu/MenuNames.h"

namespace LIBC_NAMESPACE_DECL
{
static ImGuiMouseSource ImGui_ImplWin32_GetMouseSourceFromMessageExtraInfo()
{
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700) return ImGuiMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780) return ImGuiMouseSource_TouchScreen;
    return ImGuiMouseSource_Mouse;
}

namespace Ime
{
bool ctrlDown = false;

void ImeMenu::PostDisplay()
{
    ImeApp::GetInstance().Render();
}

auto ImeMenu::ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS
{
    void();
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
                if (ProcessScaleformEvent(scaleformData))
                {
                    return RE::UI_MESSAGE_RESULTS::kHandled;
                }
            }
        }
        break;
        default:;
    }
    return IMenu::ProcessMessage(a_message);
}

void ImeMenu::OnShow()
{
    log_debug("ToolWindowMenu: Show");
    m_fSShow = true;
}

void ImeMenu::OnHide()
{
    log_debug("ToolWindowMenu: Hide");
    m_fSShow = false;
}

bool ImeMenu::ProcessScaleformEvent(const RE::BSUIScaleformData *data)
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
        case RE::GFxEvent::EventType::kCharEvent:
            return OnCharEvent(fxEvent);
        case static_cast<RE::GFxEvent::EventType>(GFxEventTypeEx::kImeCharEvent):
            fxEvent->type = RE::GFxEvent::EventType::kCharEvent;
            break;
        default:
            break;
    }
    return false;
}

bool IsKeyWillTriggerIme(const RE::GFxKey::Code keycode)
{
    return keycode >= RE::GFxKey::kA && keycode <= RE::GFxKey::kZ;
}

bool ImeMenu::OnKeyEvent(RE::GFxEvent *event, const bool down)
{
    const auto keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);
    if (keyEvent->keyCode == RE::GFxKey::kControl)
    {
        ctrlDown = down;
    }

    const auto &state = Core::State::GetInstance();
    if (state.NotHas(Core::State::LANG_PROFILE_ACTIVATED) || Utils::IsModifierDown() || Utils::IsCapsLockOn())
    {
        return false;
    }

    if (state.IsImeInputting())
    {
        return true;
    }

    if (state.IsImeWaitingInput() && IsKeyWillTriggerIme(keyEvent->keyCode))
    {
        return true;
    }
    return false;
}

bool ImeMenu::OnMouseEvent(RE::GFxEvent *event, bool down)
{
    const auto &mouseSource = ImGui_ImplWin32_GetMouseSourceFromMessageExtraInfo();
    const auto *mouseEvent  = reinterpret_cast<RE::GFxMouseEvent *>(event);
    auto       &io          = ImGui::GetIO();

    io.AddMouseSourceEvent(mouseSource);
    io.AddMouseButtonEvent(static_cast<int>(mouseEvent->button), down);

    if (Core::State::GetInstance().IsImeInputting())
    {
        if (!io.WantCaptureMouse)
        {
            // abort ime when click area is not ImeMenu when inputting;
            ImeApp::GetInstance().GetImeWnd().AbortIme();
        }
        return true; // avoid underlying menu losing input focus;
    }
    return false;
}

bool ImeMenu::OnMouseWheelEvent(RE::GFxEvent *event)
{
    const auto *mouseEvent = reinterpret_cast<RE::GFxMouseEvent *>(event);
    ImGui::GetIO().AddMouseWheelEvent(0, mouseEvent->scrollDelta);
    return false;
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
                RE::InterfaceStrings::GetSingleton()->console,
                RE::UI_MESSAGE_TYPE::kScaleformEvent,
                pScaleFormMessageData
            );
            return true;
        }
    }

    return false;
}

bool ImeMenu::OnCharEvent(RE::GFxEvent *event)
{
    const auto charEvent = reinterpret_cast<RE::GFxCharEvent *>(event);
    if (IsPaste(charEvent))
    {
        return SendFakeControlUpEvent() && Paste();
    }

    const auto &state = Core::State::GetInstance();
    if (state.NotHas(Core::State::LANG_PROFILE_ACTIVATED) || Utils::IsModifierDown() || Utils::IsCapsLockOn())
    {
        return false;
    }

    if (state.IsImeInputting())
    {
        return true;
    }
    if (state.IsImeWaitingInput() && ((charEvent->wcharCode > 'a' && charEvent->wcharCode < 'z') ||
                                      (charEvent->wcharCode > 'A' && charEvent->wcharCode < 'Z')))
    {
        return true;
    }
    return false;
}

bool ImeMenu::IsPaste(const RE::GFxCharEvent *charEvent)
{
    return charEvent->wcharCode == 'v' && ctrlDown;
}

// We only handle CF_UNICODETEXT here.
// If the clipboard only provides CF_TEXT (ANSI),
// let the game handle paste natively.
// This avoids incorrect Unicode injection and
// preserves vanilla behavior.
bool ImeMenu::Paste()
{
    if (::OpenClipboard(nullptr) == FALSE)
    {
        return false;
    }

    if (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE)
    {
        ::CloseClipboard();
        return false;
    }

    if (const HANDLE handle = ::GetClipboardData(CF_UNICODETEXT); handle != nullptr)
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
    log_info("Registering ToolWindowMenu...");
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
}
}