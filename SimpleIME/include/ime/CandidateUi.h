#ifndef IME_IME_H
#define IME_IME_H

#pragma once

#include <list>
#include <shared_mutex>
#include <string>
#include <vector>
#include <windows.h>

namespace Ime
{
struct CandWindowProp
{
    static constexpr std::uint8_t MAX_COUNT         = 32;
    static constexpr std::uint8_t DEFAULT_PAGE_SIZE = 5;
    static constexpr float        PADDING           = 10.0F;
};

class CandidateUi
{
public:
    using CandidateList_t = std::vector<std::string>;
    using size_type       = CandidateList_t::size_type;

    void Reserve(const DWORD dwPageSize) { m_candidateList.reserve(dwPageSize); }

    //! [internal use] A cache field. Be used to the candidate list is need to update when page up or page down.
    void SetFirstIndex(const DWORD firstIndex) { m_dwFirstIndex = firstIndex; }

    void SetSelection(const DWORD dwSelection) { m_dwSelection = dwSelection; }

    [[nodiscard]] auto FirstIndex() const -> DWORD { return m_dwFirstIndex; }

    [[nodiscard]] auto Selection() const -> DWORD { return m_dwSelection; }

    [[nodiscard]] auto CandidateList() const -> CandidateList_t { return m_candidateList; }

    auto PushBack(const std::string &candidate) -> void { m_candidateList.push_back(candidate); }

    auto PushBack(std::string &&candidate) -> void { m_candidateList.emplace_back(std::move(candidate)); }

    auto swap(CandidateUi &right) -> void
    {
        m_candidateList.swap(right.m_candidateList);
        std::swap(m_dwFirstIndex, right.m_dwFirstIndex);
        std::swap(m_dwSelection, right.m_dwSelection);
    }

    auto empty() -> bool { return m_candidateList.empty(); }

    /**
     * Close current candidate list
     */
    void Close() { m_candidateList.clear(); }

private:
    CandidateList_t m_candidateList;
    DWORD           m_dwFirstIndex{0}; ///< [internal use] the first index of current candidate list. Be used to determine is page up or page down.
    DWORD           m_dwSelection{0};
};
} // namespace Ime

#endif
