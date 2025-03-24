#pragma once

#include "SimpleImeSupport.h"
#include "core/State.h"

#include <atomic>
#include <cstdint>
#include <mutex>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImeSupportUtils
        {
            ImeSupportUtils()  = default;
            ~ImeSupportUtils() = default;
            using State        = Ime::Core::State;

        public:
            // Modex mod https://www.nexusmods.com/skyrimspecialedition/mods/137877
            static bool BroadcastImeMessage(SimpleIME::SkseImeMessage message, void *data, uint32_t dataLen);

            static bool BroadcastImeIntegrationMessage(SimpleIME::IntegrationData *api);

            static void UpdateImeWindowPosition(float posX, float posY);

            // The IME enabled state is async update.
            // Must use IsWantCaptureInput to check current IME state.
            static bool EnableIme(bool enable);

            static uint32_t PushContext();
            static uint32_t PopContext();

            /// <summary>
            //  Check current IME want to capture user keyboard input?
            //  Note: iFly won't update conversion mode value
            /// </summary>
            /// <returns>return true if SimpleIME mod enabled and IME not in alphanumeric mode,
            /// otherwise, return false.
            /// </returns>
            static bool IsWantCaptureInput();

        private:
            mutable std::mutex    m_mutex;
            std::atomic<uint32_t> m_refCount;

            static auto GetInstance() -> ImeSupportUtils &;

            static auto IsAllowAction(State &state) -> bool;
        };
    }
}