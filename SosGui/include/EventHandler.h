#pragma once

#include "SosGui.h"

enum ImGuiKey : int;

namespace LIBC_NAMESPACE_DECL
{
    class InputEventSink : public RE::BSTEventSink<RE::InputEvent *>
    {
        SosGui *m_SosGui;

    public:
        explicit InputEventSink(SosGui *sosGui) : m_SosGui(sosGui)
        {
        }

        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent *const *events,
                                              RE::BSTEventSource<RE::InputEvent *> * /*eventSource*/) override;

    private:
        static void ProcessKeyboardButtonEvent(const RE::ButtonEvent *buttonEvent);
        static void ProcessCharEvent(const RE::CharEvent *charEvent);
        static void ProcessMouseButtonEvent(const RE::ButtonEvent *buttonEvent);
        static auto DikToImGuiKey(uint32_t dikCode) -> ImGuiKey;
    };

    class EventHandler
    {
    public:
        static void InstallSink(SosGui *sosGui);
    };
}