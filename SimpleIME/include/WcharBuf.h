#pragma once

#include "configs/Configs.h"
#include <winnt.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        class WcharBuf
        {
        public:
            constexpr WcharBuf(HANDLE heap, DWORD initSize)
                : m_szStr(static_cast<LPWSTR>(HeapAlloc(heap, HEAP_GENERATE_EXCEPTIONS, initSize))),
                  m_dwCapacity(initSize), m_dwSize(0), m_heap(heap)
            {
                m_szStr[0] = '\0'; // should pass because HeapAlloc use HEAP_GENERATE_EXCEPTIONS
            }

            ~WcharBuf()                                         = default;
            WcharBuf(const WcharBuf &other)                     = delete;
            WcharBuf(WcharBuf &&other) noexcept                 = delete;
            WcharBuf      &operator=(const WcharBuf &other)     = delete;
            WcharBuf      &operator=(WcharBuf &&other) noexcept = delete;

            auto           TryReAlloc(DWORD bufLen) -> bool;

            constexpr void Clear()
            {
                m_szStr[0] = '\0';
                m_dwSize   = 0;
            }

            [[nodiscard]] constexpr auto IsEmpty() const -> bool
            {
                return m_dwSize == 0;
            }

            // Getters

            [[nodiscard]] constexpr auto Capacity() const -> DWORD
            {
                return m_dwCapacity;
            }

            [[nodiscard]] constexpr auto Size() const -> DWORD
            {
                return m_dwSize;
            }

            [[nodiscard]] constexpr auto Data() const -> LPWSTR
            {
                return m_szStr;
            }

            void SetSize(const DWORD dwSize)
            {
                m_dwSize        = dwSize;
                m_szStr[dwSize] = '\0';
            }

        private:
            LPWSTR m_szStr;
            DWORD  m_dwCapacity;
            DWORD  m_dwSize;
            HANDLE m_heap;
        };
    }
}