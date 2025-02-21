#ifndef TSF_SUPPORT_H
#define TSF_SUPPORT_H
#pragma once

#include "common/common.h"

#include <atlcomcli.h>
#include <msctf.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Tsf
    {
        std::string    ToErrorMessage(HRESULT hresult);

        constexpr void throw_fail(const HRESULT hresult, const char *msg) noexcept(false)
        {
            if (FAILED(hresult))
            {
                const auto errMsg = std::format("{} because: {}", msg, ToErrorMessage(hresult));
                throw std::runtime_error(errMsg);
            }
        }

        class TsfSupport
        {
        public:
            /**
             * Initialize TSF on current thread.
             * @param uiLessMode enable ui control by tsf app. use this flag
             * we can disable system IME candidate & read info UI
             * @return true if initialize success, otherwise false. App should interrupt the any following TSF call if
             * init failed.
             */
            HRESULT InitializeTsf(bool uiLessMode);
            TsfSupport() = default;
            ~TsfSupport();
            TsfSupport(const TsfSupport &other)                                 = delete;
            TsfSupport(TsfSupport &&other) noexcept                             = delete;
            TsfSupport                  &operator=(const TsfSupport &other)     = delete;
            TsfSupport                  &operator=(TsfSupport &&other) noexcept = delete;

            [[nodiscard]] constexpr auto GetThreadMgr() const -> const CComPtr<ITfThreadMgrEx> &
            {
                return m_pThreadMgr;
            }

            [[nodiscard]] constexpr auto GetMessagePump() -> const CComPtr<ITfMessagePump> &
            {
                if (m_messagePump == nullptr && m_pThreadMgr != nullptr)
                {
                    ATLENSURE_SUCCEEDED(m_pThreadMgr.QueryInterface(&m_messagePump));
                }
                return m_messagePump;
            }

            [[nodiscard]] constexpr auto GetKeystrokeMgr() -> const CComPtr<ITfKeystrokeMgr> &
            {
                if (m_keystrokeMgr == nullptr && m_pThreadMgr != nullptr)
                {
                    ATLENSURE_SUCCEEDED(m_pThreadMgr.QueryInterface(&m_keystrokeMgr));
                }
                return m_keystrokeMgr;
            }

            [[nodiscard]] constexpr auto GetTfClientId() const -> TfClientId
            {
                return m_tfClientId;
            }

        private:
            CComPtr<ITfThreadMgrEx>  m_pThreadMgr;
            CComPtr<ITfMessagePump>  m_messagePump;
            CComPtr<ITfKeystrokeMgr> m_keystrokeMgr;

            TfClientId               m_tfClientId{};
            bool                     m_initialized = false;
        };
    }
}

#endif
