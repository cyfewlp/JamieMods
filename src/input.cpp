#include "Context.h"
#include "imgui.h"
#include <dinput.h>

static RE::WinAPI::GUID guid_keyboard
    = { 0x6F1D2B61, 0xD5A0, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

void Input::TestInputEventSink::SendMouseEvent(RE::INPUT_DEVICE device, std::uint32_t key, float value, bool pressed)
{
    auto& io = ImGui::GetIO();
    switch (auto mouseKey = static_cast<RE::BSWin32MouseDevice::Key>(key)) {
    case RE::BSWin32MouseDevice::Key::kWheelUp:
        io.AddMouseWheelEvent(0, value);
        break;
    case RE::BSWin32MouseDevice::Key::kWheelDown:
        io.AddMouseWheelEvent(0, value * -1);
        break;
    default:
        LOG(debug, "mouse button event {}", key);
        io.AddMouseButtonEvent(mouseKey, pressed);
        break;
    }
}
RE::BSEventNotifyControl Input::TestInputEventSink::ProcessEvent(
    RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*)
{
    if (!Context::inputEventsEnabled)
        return RE::BSEventNotifyControl::kContinue;

    for (auto event = *a_evn; event; event = event->next) {
        const auto buttonEvent = event->AsButtonEvent();
        if (!buttonEvent)
            continue;

        const auto key = buttonEvent->GetIDCode();
        auto hotKey = key;

        RE::INPUT_DEVICE device = event->GetDevice();
        if (device == RE::INPUT_DEVICE::kKeyboard) {
            if (buttonEvent->IsPressed()) {
                LOG(debug, "pressed key {}", hotKey);
            }
        } else if (device == RE::INPUT_DEVICE::kMouse) {
            SendMouseEvent(device, hotKey, buttonEvent->Value(), buttonEvent->IsPressed());
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

void Input::Devices::RegisterInputEvent()
{
    if (const auto inputMgr = RE::BSInputDeviceManager::GetSingleton()) {
        inputMgr->AddEventSink<RE::InputEvent*>(new TestInputEventSink());
        LOG(info, "Registered TestInputEventSink.");
    }
}