#include "EventHandler.h"

#include "SosGuiMenu.h"
#include "imgui.h"

#include <common/log.h>

extern ImGuiKey ImGui_ImplWin32_KeyEventToImGuiKey(WPARAM wParam, LPARAM lParam);

namespace LIBC_NAMESPACE_DECL
{
    RE::BSEventNotifyControl InputEventSink::ProcessEvent(RE::InputEvent *const *events,
                                                          RE::BSTEventSource<RE::InputEvent *> *)
    {
        if (events != nullptr)
        {
            for (auto *event = *events; event != nullptr; event = event->next)
            {
                switch (event->GetEventType())
                {
                    case RE::INPUT_EVENT_TYPE::kButton: {
                        if (auto *buttonEvent = event->AsButtonEvent(); buttonEvent != nullptr)
                        {
                            switch (buttonEvent->GetDevice())
                            {
                                case RE::INPUT_DEVICE::kKeyboard:
                                    ProcessKeyboardButtonEvent(buttonEvent);
                                    break;
                                case RE::INPUT_DEVICE ::kMouse:
                                    ProcessMouseButtonEvent(buttonEvent);
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                    }
                    case RE::INPUT_EVENT_TYPE::kChar:
                        ProcessCharEvent(event->AsCharEvent());
                        break;
                    default:
                        break;
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }
        return RE::BSEventNotifyControl();
    }

    void InputEventSink::ProcessKeyboardButtonEvent(const RE::ButtonEvent *buttonEvent)
    {
        using Keys = RE::BSWin32KeyboardDevice::Keys;
        if (buttonEvent->GetIDCode() == Keys::kF1 && buttonEvent->IsDown())
        {
            // m_SosGui->ToggleShow();
            static bool fShow  = false;
            fShow              = !fShow;
            auto *messageQueue = RE::UIMessageQueue::GetSingleton();
            messageQueue->AddMessage(SosGuiMenu::MENU_NAME,
                                     fShow ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
        auto imGuiKey = DikToImGuiKey(buttonEvent->GetIDCode());
        ImGui::GetIO().AddKeyEvent(imGuiKey, buttonEvent->IsPressed());
    }

    auto InputEventSink::DikToImGuiKey(uint32_t dikCode) -> ImGuiKey
    {
        HKL          keyboardLayout = GetKeyboardLayout(0);
        const UINT   vkCode         = ::MapVirtualKeyEx(dikCode, MAPVK_VSC_TO_VK, keyboardLayout);
        const BOOL   isExtended     = (vkCode == VK_RCONTROL || vkCode == VK_RMENU);
        const LPARAM lParam         = isExtended << 24;
        return ImGui_ImplWin32_KeyEventToImGuiKey(vkCode, lParam);
    }

    void InputEventSink::ProcessMouseButtonEvent(const RE::ButtonEvent *buttonEvent)
    {
        using Keys = RE::BSWin32MouseDevice::Keys;
        auto value = buttonEvent->Value();
        switch (auto mouseKey = buttonEvent->GetIDCode())
        {
            case Keys::kWheelUp:
                ImGui::GetIO().AddMouseWheelEvent(0, value);
                break;
            case Keys::kWheelDown:
                ImGui::GetIO().AddMouseWheelEvent(0, value * -1);
                break;
            default: {
                ImGui::GetIO().AddMouseButtonEvent(static_cast<int>(mouseKey), buttonEvent->IsPressed());
                break;
            }
        }
    }

    void InputEventSink::ProcessCharEvent(const RE::CharEvent *charEvent)
    {
        if (charEvent == nullptr)
        {
            return;
        }

        ImGui::GetIO().AddInputCharacter(charEvent->keycode);
    }

    void EventHandler::InstallSink(SosGui *sosGui)
    {
        static InputEventSink sink(sosGui);
        log_info("Installing InputEvent sink..");

        RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent *>(&sink);
    }
}