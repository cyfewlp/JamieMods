#pragma once

#include "SosGui.h"

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
        void        ProcessKeyboardButtonEvent(const RE::ButtonEvent *buttonEvent);
        static void ProcessMouseButtonEvent(const RE::ButtonEvent *buttonEvent);
    };

    class EventHandler
    {
    public:
        static void InstallSink(SosGui *sosGui);
    };
}