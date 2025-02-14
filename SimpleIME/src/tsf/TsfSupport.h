#ifndef TSF_SUPPORT_H
#define TSF_SUPPORT_H
#pragma once

#include "configs/Configs.h"
#include <atlcomcli.h>
#include <msctf.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        std::string    ToErrorMessage(HRESULT hresult);

        constexpr void throw_fail(HRESULT hresult, const char *msg) noexcept(false);

        class TsfSupport
        {
        public:
            bool InitializeTsf();
            ~TsfSupport();

            [[nodiscard]] constexpr auto GetThreadMgr() -> CComPtr<ITfThreadMgr> &
            {
                return m_pThreadMgr;
            }

            [[nodiscard]] constexpr auto GetTfClientId() -> TfClientId
            {
                return m_tfClientId;
            }

        private:
            CComPtr<ITfThreadMgr> m_pThreadMgr;
            TfClientId            m_tfClientId;
        };
    }
}

#endif
