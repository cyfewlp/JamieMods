#pragma once

#include <unordered_set>

using MapToSet = std::unordered_map<std::string, std::unordered_set<std::string>>;

namespace Transmogrify {

    void AccessPlayerInventory();

    void GetAllArmors(MapToSet* modToArmors);

    class CombineEventSinks
      : public Singleton<CombineEventSinks>
      , public RE::BSTEventSink<RE::TESActorLocationChangeEvent>
    {
        friend class Singleton<CombineEventSinks>;

    public:
        static void Install();

        RE::BSEventNotifyControl ProcessEvent(const RE::TESActorLocationChangeEvent* a_event,
                                              RE::BSTEventSource<RE::TESActorLocationChangeEvent>* a_eventSource);

    private:
        CombineEventSinks() = default;
        ~CombineEventSinks() = default;
    };

} // namespace Transmogrify