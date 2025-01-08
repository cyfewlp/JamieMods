
namespace Transmogrify {

    void AccessPlayerInventory();

    class CombineEventSinks : public RE::BSTEventSink<RE::TESActorLocationChangeEvent> {
    public:
        ~CombineEventSinks() = default;
        CombineEventSinks(const CombineEventSinks&) = delete;
        CombineEventSinks(CombineEventSinks&&) = delete;
        CombineEventSinks& operator=(const CombineEventSinks&) = delete;
        CombineEventSinks& operator=(CombineEventSinks&&) = delete;

        static auto GetSingleton() -> CombineEventSinks*;

        static void Install();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* a_event,
            RE::BSTEventSource<RE::TESActorLocationChangeEvent>* a_eventSource);

    private:
        CombineEventSinks() = default;
    };

} // namespace Transmogrify