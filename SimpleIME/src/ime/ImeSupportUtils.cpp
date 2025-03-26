#include "ime/ImeSupportUtils.h"
#include "SimpleImeSupport.h"
#include "core/State.h"
#include "ime/ImeManager.h"
#include "imgui.h"

#include <SKSE/API.h>
#include <common/config.h>
#include <common/log.h>
#include <cstdint>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        bool ImeSupportUtils::BroadcastImeMessage(SimpleIME::SkseImeMessage message, void *data, uint32_t dataLen)
        {
            log_debug("Send Message {}", static_cast<uint32_t>(message));
            return SKSE::GetMessagingInterface()->Dispatch(static_cast<uint32_t>(message), data, dataLen, nullptr);
        }

        bool ImeSupportUtils::BroadcastImeIntegrationMessage(SimpleIME::IntegrationData *api)
        {
            return BroadcastImeMessage(SimpleIME::SkseImeMessage::IME_INTEGRATION_INIT, reinterpret_cast<void *>(api),
                                       sizeof(SimpleIME::IntegrationData *));
        }

        void ImeSupportUtils::UpdateImeWindowPosition(float posX, float posY)
        {
            auto windowName = SKSE::PluginDeclaration::GetSingleton()->GetName();
            ImGui::SetWindowPos(windowName.data(), {posX, posY});
        }

        bool ImeSupportUtils::EnableIme(bool enable)
        {
            const std::unique_lock lockGuard(g_mutex);
            if (State::GetInstance().IsSupportOtherMod() == enable)
            {
                return false;
            }
            auto *manager = ImeManagerComposer::GetInstance();
            if (manager->FocusManageType() != FocusManageType::Temporary && enable &&
                State::GetInstance().NotHas(State::IME_DISABLED))
            {
                log_debug("Received enable IME message by other mod, try disable current IME.");
                manager->WaitEnableIme(false); // Sync wait IME close.
            }

            State::GetInstance().SetSupportOtherMod(enable);

            return manager->GetTemporaryFocusImeManager()->NotifyEnableIme(enable);
        }

        bool ImeSupportUtils::IsWantCaptureInput()
        {
            auto &state = Core::State::GetInstance();

            return state.IsModEnabled() && state.NotHas(Core::State::IME_DISABLED, Core::State::IN_ALPHANUMERIC) &&
                   state.Has(Core::State::LANG_PROFILE_ACTIVATED);
        }
    }
}