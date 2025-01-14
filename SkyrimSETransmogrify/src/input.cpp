#include "input.h"
#include "RenderManager.h"
#include "imgui.h"
#include <dinput.h>

typedef RE::BSWin32KeyboardDevice::Key KEY;
ImGuiKey ToImGuiKey(KEY);

namespace Input = Transmogrify::Input;

void
Input::ImGuiInputEventSink::SendMouseEvent(std::uint32_t key, float value, bool pressed)
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
            io.AddMouseButtonEvent(mouseKey, pressed);
            break;
    }
}

void
Input::ImGuiInputEventSink::SendKeyBoardEvent(std::uint32_t key, float value, bool pressed)
{
    ImGuiKey imguiKey{ ImGuiKey_None };
    imguiKey = ToImGuiKey(static_cast<KEY>(key));
    ImGui::GetIO().AddKeyEvent(imguiKey, pressed);
}

RE::BSEventNotifyControl
Input::ImGuiInputEventSink::ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*)
{
    if (!inputEventsEnabled)
        return RE::BSEventNotifyControl::kContinue;

    for (auto event = *a_evn; event; event = event->next) {
        const auto buttonEvent = event->AsButtonEvent();
        if (!buttonEvent)
            continue;

        const auto key = buttonEvent->GetIDCode();
        auto hotKey = key;

        RE::INPUT_DEVICE device = event->GetDevice();
        switch (device) {
            case RE::INPUT_DEVICE::kKeyboard:
                if (buttonEvent->GetIDCode() == 60 /*F2*/ && buttonEvent->IsDown()) {
                    RenderManager::SwitchShowWindow();
                }

                SendKeyBoardEvent(hotKey, buttonEvent->Value(), buttonEvent->IsPressed());
                break;
            case RE::INPUT_DEVICE::kMouse:
                SendMouseEvent(hotKey, buttonEvent->Value(), buttonEvent->IsPressed());
                break;
            default:
                break;
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

void
Input::Install()
{
    RealDispatchInputEventFunc = Hooks::DispatchInputEventHook::Install(&DispatchInputEvent);
}

void
Input::DispatchInputEvent(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_evns)
{
    static RE::InputEvent* dummy[] = { nullptr };
    if (!a_evns) {
        RealDispatchInputEventFunc(a_dispatcher, a_evns);
        return;
    }
    ImGuiInputEventSink::GetSingleton().ProcessEvent(a_evns, nullptr);
    if (RenderManager::IsShowWindow()) {
        RealDispatchInputEventFunc(a_dispatcher, dummy);
        return;
    } else {
        RealDispatchInputEventFunc(a_dispatcher, a_evns);
    }
}

ImGuiKey
ToImGuiKey(KEY a_key)
{
    switch (a_key) {
        case KEY::kTab:
            return ImGuiKey_Tab;
        case KEY::kLeft:
            return ImGuiKey_LeftArrow;
        case KEY::kRight:
            return ImGuiKey_RightArrow;
        case KEY::kUp:
            return ImGuiKey_UpArrow;
        case KEY::kDown:
            return ImGuiKey_DownArrow;
        case KEY::kPageUp:
            return ImGuiKey_PageUp;
        case KEY::kPageDown:
            return ImGuiKey_PageDown;
        case KEY::kHome:
            return ImGuiKey_Home;
        case KEY::kEnd:
            return ImGuiKey_End;
        case KEY::kInsert:
            return ImGuiKey_Insert;
        case KEY::kDelete:
            return ImGuiKey_Delete;
        case KEY::kBackspace:
            return ImGuiKey_Backspace;
        case KEY::kSpacebar:
            return ImGuiKey_Space;
        case KEY::kEnter:
            return ImGuiKey_Enter;
        case KEY::kEscape:
            return ImGuiKey_Escape;
        case KEY::kApostrophe:
            return ImGuiKey_Apostrophe;
        case KEY::kComma:
            return ImGuiKey_Comma;
        case KEY::kMinus:
            return ImGuiKey_Minus;
        case KEY::kPeriod:
            return ImGuiKey_Period;
        case KEY::kSlash:
            return ImGuiKey_Slash;
        case KEY::kSemicolon:
            return ImGuiKey_Semicolon;
        case KEY::kEquals:
            return ImGuiKey_Equal;
        case KEY::kBracketLeft:
            return ImGuiKey_LeftBracket;
        case KEY::kBackslash:
            return ImGuiKey_Backslash;
        case KEY::kBracketRight:
            return ImGuiKey_RightBracket;
        case KEY::kTilde:
            return ImGuiKey_GraveAccent;
        case KEY::kCapsLock:
            return ImGuiKey_CapsLock;
        case KEY::kScrollLock:
            return ImGuiKey_ScrollLock;
        case KEY::kNumLock:
            return ImGuiKey_NumLock;
        case KEY::kPrintScreen:
            return ImGuiKey_PrintScreen;
        case KEY::kPause:
            return ImGuiKey_Pause;
        case KEY::kKP_0:
            return ImGuiKey_Keypad0;
        case KEY::kKP_1:
            return ImGuiKey_Keypad1;
        case KEY::kKP_2:
            return ImGuiKey_Keypad2;
        case KEY::kKP_3:
            return ImGuiKey_Keypad3;
        case KEY::kKP_4:
            return ImGuiKey_Keypad4;
        case KEY::kKP_5:
            return ImGuiKey_Keypad5;
        case KEY::kKP_6:
            return ImGuiKey_Keypad6;
        case KEY::kKP_7:
            return ImGuiKey_Keypad7;
        case KEY::kKP_8:
            return ImGuiKey_Keypad8;
        case KEY::kKP_9:
            return ImGuiKey_Keypad9;
        case KEY::kKP_Decimal:
            return ImGuiKey_KeypadDecimal;
        case KEY::kKP_Divide:
            return ImGuiKey_KeypadDivide;
        case KEY::kKP_Multiply:
            return ImGuiKey_KeypadMultiply;
        case KEY::kKP_Subtract:
            return ImGuiKey_KeypadSubtract;
        case KEY::kKP_Plus:
            return ImGuiKey_KeypadAdd;
        case KEY::kKP_Enter:
            return ImGuiKey_KeypadEnter;
        case KEY::kLeftShift:
            return ImGuiKey_LeftShift;
        case KEY::kLeftControl:
            return ImGuiKey_LeftCtrl;
        case KEY::kLeftAlt:
            return ImGuiKey_LeftAlt;
        case KEY::kLeftWin:
            return ImGuiKey_LeftSuper;
        case KEY::kRightShift:
            return ImGuiKey_RightShift;
        case KEY::kRightControl:
            return ImGuiKey_RightCtrl;
        case KEY::kRightAlt:
            return ImGuiKey_RightAlt;
        case KEY::kRightWin:
            return ImGuiKey_RightSuper;
        /*case KEY::kAPPS:
            return ImGuiKey_Menu; - doesn't fire*/
        case KEY::kNum0:
            return ImGuiKey_0;
        case KEY::kNum1:
            return ImGuiKey_1;
        case KEY::kNum2:
            return ImGuiKey_2;
        case KEY::kNum3:
            return ImGuiKey_3;
        case KEY::kNum4:
            return ImGuiKey_4;
        case KEY::kNum5:
            return ImGuiKey_5;
        case KEY::kNum6:
            return ImGuiKey_6;
        case KEY::kNum7:
            return ImGuiKey_7;
        case KEY::kNum8:
            return ImGuiKey_8;
        case KEY::kNum9:
            return ImGuiKey_9;
        case KEY::kA:
            return ImGuiKey_A;
        case KEY::kB:
            return ImGuiKey_B;
        case KEY::kC:
            return ImGuiKey_C;
        case KEY::kD:
            return ImGuiKey_D;
        case KEY::kE:
            return ImGuiKey_E;
        case KEY::kF:
            return ImGuiKey_F;
        case KEY::kG:
            return ImGuiKey_G;
        case KEY::kH:
            return ImGuiKey_H;
        case KEY::kI:
            return ImGuiKey_I;
        case KEY::kJ:
            return ImGuiKey_J;
        case KEY::kK:
            return ImGuiKey_K;
        case KEY::kL:
            return ImGuiKey_L;
        case KEY::kM:
            return ImGuiKey_M;
        case KEY::kN:
            return ImGuiKey_N;
        case KEY::kO:
            return ImGuiKey_O;
        case KEY::kP:
            return ImGuiKey_P;
        case KEY::kQ:
            return ImGuiKey_Q;
        case KEY::kR:
            return ImGuiKey_R;
        case KEY::kS:
            return ImGuiKey_S;
        case KEY::kT:
            return ImGuiKey_T;
        case KEY::kU:
            return ImGuiKey_U;
        case KEY::kV:
            return ImGuiKey_V;
        case KEY::kW:
            return ImGuiKey_W;
        case KEY::kX:
            return ImGuiKey_X;
        case KEY::kY:
            return ImGuiKey_Y;
        case KEY::kZ:
            return ImGuiKey_Z;
        case KEY::kF1:
            return ImGuiKey_F1;
        case KEY::kF2:
            return ImGuiKey_F2;
        case KEY::kF3:
            return ImGuiKey_F3;
        case KEY::kF4:
            return ImGuiKey_F4;
        case KEY::kF5:
            return ImGuiKey_F5;
        case KEY::kF6:
            return ImGuiKey_F6;
        case KEY::kF7:
            return ImGuiKey_F7;
        case KEY::kF8:
            return ImGuiKey_F8;
        case KEY::kF9:
            return ImGuiKey_F9;
        case KEY::kF10:
            return ImGuiKey_F10;
        case KEY::kF11:
            return ImGuiKey_F11;
        case KEY::kF12:
            return ImGuiKey_F12;
        default:
            return ImGuiKey_None;
    }
}
