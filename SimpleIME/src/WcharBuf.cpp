#include "WcharBuf.h"

namespace LIBC_NAMESPACE_DECL
{
    auto Ime::WcharBuf::TryReAlloc(DWORD bufLen) -> bool
    {
        if (bufLen > m_dwCapacity)
        {
            LPVOID hMem = static_cast<LPWSTR>(HeapReAlloc(m_heap, 0, m_szStr, bufLen));
            if (hMem == nullptr)
            {
                log_error("Try re-alloc to {} failed", bufLen);
                return false;
            }
            m_dwCapacity = bufLen;
            m_szStr      = static_cast<LPWSTR>(hMem);
        }
        return true;
    }
}