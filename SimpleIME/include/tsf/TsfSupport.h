#ifndef TSF_TSFSUPPORT_H
#define TSF_TSFSUPPORT_H
#pragma once

#include <atlcomcli.h>
#include <msctf.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Tsf
{
auto ToErrorMessage(HRESULT hresult) -> std::string;

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
    auto InitializeTsf(bool uiLessMode) -> HRESULT;
    TsfSupport() = default;
    ~TsfSupport();
    TsfSupport(const TsfSupport &other)                         = delete;
    TsfSupport(TsfSupport &&other) noexcept                     = delete;
    auto operator=(const TsfSupport &other) -> TsfSupport &     = delete;
    auto operator=(TsfSupport &&other) noexcept -> TsfSupport & = delete;

    [[nodiscard]] constexpr auto GetThreadMgr() const -> const CComPtr<ITfThreadMgrEx> &
    {
        return m_pThreadMgr;
    }

    [[nodiscard]] constexpr auto GetMessagePump() const -> CComPtr<ITfMessagePump>
    {
        return m_messagePump;
    }

    [[nodiscard]] constexpr auto GetKeystrokeMgr() const -> CComPtr<ITfKeystrokeMgr>
    {
        return m_keystrokeMgr;
    }

    [[nodiscard]] constexpr auto GetTfClientId() const -> const TfClientId &
    {
        return m_tfClientId;
    }

    static auto GetSingleton(bool uiLessMode = true) -> TsfSupport const &
    {
        if (!s_instance.m_initialized)
        {
            s_instance.InitializeTsf(uiLessMode);
        }
        return s_instance;
    }

private:
    CComPtr<ITfThreadMgrEx>  m_pThreadMgr;
    CComPtr<ITfMessagePump>  m_messagePump;
    CComPtr<ITfKeystrokeMgr> m_keystrokeMgr;

    TfClientId m_tfClientId{};
    bool       m_initialized = false;

    static TsfSupport s_instance;
};
} // namespace Tsf
} // namespace LIBC_NAMESPACE_DECL

#endif
