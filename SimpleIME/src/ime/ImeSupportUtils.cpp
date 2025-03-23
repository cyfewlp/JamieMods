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

        //////////////////////////////////////////////////////////////////////////
        // Support methods
        //////////////////////////////////////////////////////////////////////////

        void ImeSupportUtils::UpdateImeWindowPosition(float posX, float posY)
        {
            if (GetInstance().IsAllowAction(State::GetInstance()))
            {
                auto windowName = SKSE::PluginDeclaration::GetSingleton()->GetName();
                ImGui::SetWindowPos(windowName.data(), {posX, posY});
            }
        }

        bool ImeSupportUtils::EnableIme(bool enable)
        {
            const std::unique_lock lockGuard(GetInstance().m_mutex);

            auto &state   = State::GetInstance();
            bool  success = GetInstance().IsAllowAction(state);
            if (success)
            {
                if ((enable && state.Has(State::IME_DISABLED)) || (!enable && state.NotHas(State::IME_DISABLED)))
                {
                    log_debug("ImeSupportUtils {} IME", enable ? "enable" : "disable");
                    success = ImeManagerComposer::GetInstance()->NotifyEnableIme(enable);
                }
            }

            return success;
        }

        uint32_t ImeSupportUtils::PushContext()
        {
            const std::unique_lock lockGuard(GetInstance().m_mutex);

            auto &instance = GetInstance();
            auto  prev     = instance.m_refCount.load();
            if (instance.m_refCount >= 0 && instance.m_refCount++ == 0)
            {
                State::GetInstance().SetSupportOtherMod(true);
            }
            return prev;
        }

        uint32_t ImeSupportUtils::PopContext()
        {
            const std::unique_lock lockGuard(GetInstance().m_mutex);

            auto &instance = GetInstance();
            auto  prev     = instance.m_refCount.load();
            if (instance.m_refCount > 0 && --instance.m_refCount == 0)
            {
                State::GetInstance().SetSupportOtherMod(false);
                ImeManagerComposer::GetInstance()->SyncImeState();
            }
            return prev;
        }

        bool ImeSupportUtils::IsWantCaptureInput()
        {
            auto &state = Core::State::GetInstance();

            return GetInstance().IsAllowAction(state) &&
                   state.NotHas(Core::State::IME_DISABLED, Core::State::IN_ALPHANUMERIC) &&
                   state.Has(Core::State::LANG_PROFILE_ACTIVATED);
        }

        auto ImeSupportUtils::GetInstance() -> ImeSupportUtils &
        {
            static ImeSupportUtils g_instance;
            return g_instance;
        }

        auto ImeSupportUtils::IsAllowAction(State &state) const -> bool
        {
            return state.IsModEnabled() && state.IsSupportOtherMod();
        }
    }
}