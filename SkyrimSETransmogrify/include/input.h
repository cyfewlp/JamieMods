#pragma once

#include "Hooks.h"
namespace Transmogrify {
    namespace Input {
        static REL::Relocation<Hooks::DispatchInputEventHook::RealFunc> RealDispatchInputEventFunc;
        static std::atomic<bool> inputEventsEnabled = true;

        void Install();

        static void DispatchInputEvent(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_evns);

        class ImGuiInputEventSink
          : public Singleton<ImGuiInputEventSink>
          , public RE::BSTEventSink<RE::InputEvent*>
        {
            friend class Singleton<ImGuiInputEventSink>;

        public:
            RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_evn,
                                                  RE::BSTEventSource<RE::InputEvent*>*) override;

        private:
            void SendMouseEvent(std::uint32_t key, float value, bool pressed);
            void SendKeyBoardEvent(std::uint32_t key, float value, bool pressed);
        };
    }
};