#pragma once

#include "common/common.h"

#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        struct CandWindowProp
        {
            static constexpr std::uint8_t MAX_COUNT         = 32;
            static constexpr std::uint8_t DEFAULT_PAGE_SIZE = 5;
            static constexpr float        PADDING           = 10.0F;
        };

        struct CandidateUi
        {
            CandidateUi()                                        = default;
            ~CandidateUi()                                       = default;
            CandidateUi(const CandidateUi &other)                = delete;
            CandidateUi(CandidateUi &&other) noexcept            = delete;
            CandidateUi &operator=(const CandidateUi &other)     = delete;
            CandidateUi &operator=(CandidateUi &&other) noexcept = delete;

            void         SetPageSize(const DWORD dwPageSize)
            {
                m_dwPageSize = dwPageSize == 0U ? CandWindowProp::DEFAULT_PAGE_SIZE : dwPageSize;
            }

            void SetSelection(const DWORD dwSelection)
            {
                m_dwSelection = dwSelection % m_dwPageSize;
            }

            [[nodiscard]] constexpr auto PageSize() const -> DWORD
            {
                return m_dwPageSize;
            }

            [[nodiscard]] constexpr auto Selection() const -> DWORD
            {
                return m_dwSelection;
            }

            [[nodiscard]] constexpr auto CandidateList() const -> const std::list<std::string> &
            {
                return m_candidateList;
            }

            auto PushBack(const std::string &candidate) -> void
            {
                m_candidateList.push_back(candidate);
            }

            /**
             * Close current candidate list
             */
            void Close()
            {
                m_dwSelection = 0;
                m_candidateList.clear();
                m_dwPageSize = CandWindowProp::DEFAULT_PAGE_SIZE;
            }

        private:
            DWORD                  m_dwPageSize{CandWindowProp::DEFAULT_PAGE_SIZE};
            DWORD                  m_dwSelection{0};
            std::list<std::string> m_candidateList;
        } ALIGN(64);
    }
}