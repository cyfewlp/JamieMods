#include "EventHandler.h"

#include "imgui.h"

#include <common/log.h>

namespace LIBC_NAMESPACE_DECL
{
    RE::BSEventNotifyControl InputEventSink::ProcessEvent(RE::InputEvent *const *events,
                                                          RE::BSTEventSource<RE::InputEvent *> *)
    {
        if (events != nullptr)
        {
            for (auto *event = *events; event != nullptr; event = event->next)
            {
                if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton)
                {
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
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }
        return RE::BSEventNotifyControl();
    }

    void InputEventSink::ProcessKeyboardButtonEvent(const RE::ButtonEvent *buttonEvent)
    {
        using Keys = RE::BSWin32KeyboardDevice::Keys;
        if (buttonEvent->GetIDCode() == Keys::kF5)
        {
            m_SosGui->ToggleShow();
        }
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

    void EventHandler::InstallSink(SosGui *sosGui)
    {
        static InputEventSink sink(sosGui);
        log_info("Installing InputEvent sink..");;
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent *>(&sink);
    }
}