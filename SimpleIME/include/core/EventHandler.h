#pragma once

namespace LIBC_NAMESPACE_DECL
{

namespace Ime
{
struct Settings;
class ImeWnd;
}

namespace Ime::Core
{
class InputEventSink : public RE::BSTEventSink<RE::InputEvent *>
{
    using Event = RE::InputEvent;
    using Keys  = RE::BSWin32MouseDevice::Keys;

public:
    explicit InputEventSink(ImeWnd *m_imeWnd) : m_imeWnd(m_imeWnd) {}

    RE::BSEventNotifyControl ProcessEvent(Event *const *event, RE::BSTEventSource<Event *> * /*eventSource*/) override;

private:
    ImeWnd *m_imeWnd;

    void ProcessMouseButtonEvent(const RE::ButtonEvent *buttonEvent) const;
    void ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent) const;
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
    static auto UpdateMessageFilter(const Settings &settings, RE::InputEvent **a_events) -> void;

private:
    static constexpr auto IsPasteShortcutPressed(auto &code);
};
}
}