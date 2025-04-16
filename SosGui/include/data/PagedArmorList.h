#pragma once

#include "common/config.h"

#if !defined(NDEBUG)
    #define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
    #define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index_container.hpp>
#include <cstdint>
#include <stdexcept>
#include <string.h>
#include <unordered_set>

namespace LIBC_NAMESPACE_DECL
{
    struct ArmorCompare
    {
        bool operator()(RE::TESObjectARMO *lhs, RE::TESObjectARMO *rhs) const
        {
            return strcmp(lhs->GetName(), rhs->GetName()) < 0;
        }
    };

    using namespace boost::multi_index;

    class PagedArmorList
    {
        uint16_t m_pageSize;
        uint32_t m_currentPage = 0;
        uint32_t m_pageCount   = 0;
        bool     m_fAscend     = true;

        using Armor = RE::TESObjectARMO;

    public:
        struct ArmorWrap
        {
            Armor *armorPtr = nullptr;

            ArmorWrap(Armor *armorPtr) : armorPtr(armorPtr) {}

            template <typename Slot>
            auto HasAnyPartOf(Slot &&slot) const -> bool
            {
                return armorPtr->bipedModelData.bipedObjectSlots.any(slot);
            }

            auto GetName() const -> const char * { return armorPtr->GetName(); }

            auto GetFormID() const -> RE::FormID { return armorPtr->GetFormID(); }

            [[nodiscard]] inline friend bool operator==(const ArmorWrap &a_lhs, const ArmorWrap &a_rhs) noexcept
            {
                return a_lhs.GetFormID() == a_rhs.GetFormID();
            }

            struct Hash
            {
                std::size_t operator()(const ArmorWrap &armor) const noexcept
                {
                    return armor.armorPtr == nullptr ? 0 : static_cast<std::size_t>(armor.armorPtr->GetFormID());
                }
            };
        };

        typedef boost::multi_index_container< //
            ArmorWrap, indexed_by<            //
                           ordered_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(ArmorWrap, RE::FormID, GetFormID)>,
                           ranked_non_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(ArmorWrap, const char *, GetName)>>>
            Container;

        typedef std::unordered_set<ArmorWrap, ArmorWrap::Hash> UnusedArmor;
        typedef nth_index<Container, 0>::type                  ContainerByFormId;
        typedef nth_index<Container, 1>::type                  ContainerByName;

        using FormIdIterator        = ContainerByFormId::iterator;
        using ReverseFormIdIterator = ContainerByFormId::reverse_iterator;
        using NameIterator          = ContainerByName::iterator;
        using ReverseNameIterator   = ContainerByName::reverse_iterator;

    private:
        Container          m_Container;
        UnusedArmor        m_unusedArmor;
        ContainerByFormId &m_containerByFormId = get<0>(m_Container);
        ContainerByName   &m_containerByName   = get<1>(m_Container);

        void UpdatePageInfo()
        {
            m_pageCount = m_Container.size() / m_pageSize + 1;
            if (m_currentPage >= m_pageCount) { m_currentPage = std::max(0U, m_pageCount - 1); }
        }

    public:
        struct invalid_page_size : std::runtime_error
        {
            explicit invalid_page_size() : runtime_error("Invalid Page Size 0") {}
        };

        PagedArmorList(uint16_t pageSize) : m_pageSize(pageSize)
        {
            if (pageSize == 0) { throw invalid_page_size(); }
        }

        ~PagedArmorList() = default;

        void Insert(const ArmorWrap &armorWrap, bool isUsed = false)
        {
            FormIdIterator iter = m_Container.cend();
            if (armorWrap.armorPtr == nullptr) { return; }
            if (isUsed) { m_Container.insert(armorWrap); }
            else
            {
                m_Container.erase(armorWrap.armorPtr->GetFormID());
                m_unusedArmor.insert(armorWrap);
            }
            UpdatePageInfo();
        }

        void Insert(std::vector<ArmorWrap> armorList, bool isUsed = false)
        {
            if (isUsed)
            {
                m_Container.insert(armorList.begin(), armorList.end());
                for (const auto &armorWrap : armorList)
                {
                    m_unusedArmor.erase(armorWrap);
                }
            }
            else
            {
                for (const auto &armorWrap : armorList)
                {
                    m_Container.erase(armorWrap.GetFormID());
                    m_unusedArmor.insert(armorWrap);
                }
            }
            UpdatePageInfo();
        }

        void Insert(Armor *armor, bool isUsed = false) { Insert(ArmorWrap(armor), isUsed); }

        void RestoreUnusedArmors()
        {
            for (auto itBegin = m_unusedArmor.begin(); itBegin != m_unusedArmor.end();)
            {
                m_Container.insert(*itBegin);
                itBegin = m_unusedArmor.erase(itBegin);
            }
            UpdatePageInfo();
        }

        void Remove(Armor *armor)
        {
            if (armor == nullptr) { return; }
            m_unusedArmor.insert(armor);
            m_Container.erase(armor->GetFormID());
            UpdatePageInfo();
        }

        void Clear()
        {
            m_Container.clear();
            m_unusedArmor.clear();
            m_currentPage = 0;
            m_pageCount   = 0;
        }

        constexpr void SetSortDirection(bool ascend) { m_fAscend = ascend; }

        constexpr auto IsEmpty() const -> bool { return m_Container.empty(); }

        constexpr auto Size() const -> size_t { return m_Container.size(); }

        // iterator function

        auto begin() -> FormIdIterator { return m_Container.begin(); }

        auto end() -> FormIdIterator { return m_Container.end(); }

        /// Page Function

        [[nodiscard]] auto GetPageSize() const -> uint16_t { return m_pageSize; }

        [[nodiscard]] auto GetPageIndex() const -> uint32_t { return m_currentPage + 1; }

        [[nodiscard]] auto GetPageCount() const -> uint32_t { return m_pageCount; }

        void SetPageSize(uint16_t newPageSize)
        {
            if (newPageSize == 0) { throw invalid_page_size(); }
            m_pageSize = newPageSize;
            UpdatePageInfo();
        }

        void SetCurrentPage(uint32_t pageIndex)
        {
            if (pageIndex >= m_pageCount) { throw invalid_page_size(); }
            m_currentPage = 0;
        }

        void PrevPage()
        {
            if (m_currentPage > 0) { m_currentPage--; }
        }

        void NextPage()
        {
            if (m_currentPage < m_pageCount) { m_currentPage++; }
        }

        bool HasPrevPage() const { return m_pageCount > 1 && m_currentPage > 0; }

        bool HasNextPage() const { return m_currentPage < m_pageCount - 1; }

        auto GetAllArmorCount() const -> uint32_t { return m_unusedArmor.size() + m_Container.size(); }

        // Auto skip unused armors
        template <typename Func>
        void ForEachPage(Func &&func)
        {
            size_t start = m_currentPage * m_pageSize;
            if (start >= m_containerByName.size()) return;

            if (m_fAscend)
            {
                auto itBegin = m_containerByName.nth(start);
                for (size_t index = 0; index < m_pageSize && itBegin != m_containerByName.end(); ++itBegin)
                {
                    func(index, (*itBegin).armorPtr);
                    ++index;
                }
            }
            else
            {
                start            = m_containerByName.size() - start;
                size_t available = std::min(static_cast<size_t>(m_pageSize), start);
                auto   itBegin   = boost::make_reverse_iterator(m_containerByName.nth(start));
                auto   itEnd     = boost::make_reverse_iterator(m_containerByName.nth(start - available));
                for (size_t index = 0; index < m_pageSize && itBegin != itEnd; ++itBegin)
                {
                    func(index, (*itBegin).armorPtr);
                    ++index;
                }
            }
        }

        auto GetCurrentPage() const -> std::vector<Armor *>
        {
            size_t start = m_currentPage * m_pageSize;

            if (start >= m_containerByName.size()) { return {}; }

            std::vector<Armor *> result;
            if (m_fAscend)
            {
                auto itBegin = m_containerByName.nth(start);
                auto itEnd   = m_containerByName.nth(start + m_pageSize);
                for (auto it = itBegin; it != itEnd; ++it)
                {
                    result.push_back(it->armorPtr);
                }
            }
            else
            {
                start            = m_containerByName.size() - start;
                size_t available = std::min(static_cast<size_t>(m_pageSize), start);
                auto   itBegin   = boost::make_reverse_iterator(m_containerByName.nth(start));
                auto   itEnd     = boost::make_reverse_iterator(m_containerByName.nth(start - available));
                for (auto it = itBegin; it != itEnd; ++it)
                {
                    result.push_back(it->armorPtr);
                }
            }

            return result;
        }
    };
}