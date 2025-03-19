#pragma once

#include "SimpleImeSupport.h"
#include "core/State.h"

#include <cstdint>

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
            // Modex mod
            // https://www.nexusmods.com/skyrimspecialedition/mods/137877
            static bool BroadcastImeMessage(SimpleIME::SkseImeMessage message, void *data, uint32_t dataLen);

            static bool BroadcastImeIntegrationMessage(SimpleIME::IntegrationData *api);

            static void UpdateImeWindowPosition(float posX, float posY);
            static bool EnableIme(bool enable);
        private:

            static inline std::mutex g_mutex;
        };
    }
}