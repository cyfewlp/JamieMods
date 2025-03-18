#pragma once

namespace LIBC_NAMESPACE_DECL
{

    namespace Ime
    {
        class ImeWnd;
    }

    namespace Ime::Core
    {
        class InputEventSink : public RE::BSTEventSink<RE::InputEvent *>
        {
            using Event = RE::InputEvent;
            using Keys  = RE::BSWin32MouseDevice::Keys;

        public:
            explicit InputEventSink(ImeWnd *m_imeWnd) : m_imeWnd(m_imeWnd)
            {
            }

            RE::BSEventNotifyControl ProcessEvent(Event *const *event,
                                                  RE::BSTEventSource<Event *> * /*eventSource*/) override;

        private:
            ImeWnd *m_imeWnd;

            void ProcessMouseButtonEvent(const RE::ButtonEvent *buttonEvent);
            void ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent);
        };

        class MenuOpenCloseEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
            using Event = RE::MenuOpenCloseEvent;

        public:
            RE::BSEventNotifyControl ProcessEvent(const Event *a_event, RE::BSTEventSource<Event> *) override;

        private:
            static void FixInconsistentTextEntryCount(const Event *event);
        };

        class EventHandler
        {
            EventHandler()                                                 = delete;
            ~EventHandler()                                                = delete;
            EventHandler(const EventHandler &other)                        = delete;
            EventHandler(EventHandler &&other)                             = delete;
            EventHandler              operator=(const EventHandler &other) = delete;
            EventHandler              operator=(EventHandler &&other)      = delete;
            static constexpr uint32_t ENUM_DIK_V                           = 0x2F;
            static constexpr uint32_t ENUM_VK_CONTROL                      = 0x11;

        public:
            static void InstallEventSink(ImeWnd *imeWnd);
            static auto UpdateMessageFilter(RE::InputEvent **a_events) -> void;
            /**
             * Prevent keyboard be send when IME inputing or wait input.
             * Only detect first event.
             */
            static auto IsDiscardKeyboardEvent(const RE::ButtonEvent *buttonEvent) -> bool;
            static auto PostHandleKeyboardEvent() -> void;

        private:
            static constexpr auto IsImeNotActivateOrGameLoading() -> bool;
            static constexpr auto IsImeWantCaptureInput() -> bool;
            static constexpr auto IsPasteShortcutPressed(auto &code);
            static auto           IsWillTriggerIme(std::uint32_t code) -> bool;
        };
    }
}