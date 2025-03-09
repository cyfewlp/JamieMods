#pragma once

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime::Core
    {
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
            static auto HandleKeyboardEvent(const RE::ButtonEvent *buttonEvent) -> void;

            static auto PostHandleKeyboardEvent() -> void;

        private:
            static constexpr auto IsImeNotActivateOrGameLoading() -> bool;
            static constexpr auto IsImeWantCaptureInput() -> bool;
            static constexpr auto IsPasteShortcutPressed(auto &code);
            static auto           IsWillTriggerIme(std::uint32_t code) -> bool;
        };
    }
}