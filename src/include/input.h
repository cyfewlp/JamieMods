
namespace Input {
    class Devices {
    private:
        char keyBuffer[256];
        RE::BSWin32KeyboardDevice* pKeyboard;

    public:
        void RegisterInputEvent();
    };

    class TestInputEventSink : public RE::BSTEventSink<RE::InputEvent*> {
    private:
        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*) override;
        void SendMouseEvent(RE::INPUT_DEVICE device, std::uint32_t key, float value, bool pressed);
        void SendKeyBoardEvent(RE::INPUT_DEVICE device, std::uint32_t key, float value, bool pressed);
    };
}