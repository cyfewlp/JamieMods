#pragma once

#include "common/config.h"
#include "common/log.h"
#include "util/StringUtil.h"

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
#include <unordered_set>

namespace LIBC_NAMESPACE_DECL
{
    using namespace boost::multi_index;

    class ArmorList
    {
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

            auto GetName() const -> const char *
            {
                return armorPtr->GetName();
            }

            auto GetFormID() const -> RE::FormID
            {
                return armorPtr->GetFormID();
            }

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
            ArmorWrap,
            indexed_by< //
                ordered_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(ArmorWrap, RE::FormID, GetFormID)>,
                ranked_non_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(ArmorWrap, const char *, GetName),
                                  util::StringCompactor>>>
            Container;

        typedef std::unordered_set<ArmorWrap, ArmorWrap::Hash> UnusedArmor;
        typedef nth_index<Container, 0>::type                  ContainerByFormId;
        typedef nth_index<Container, 1>::type                  ContainerByName;

        using FormIdIterator        = ContainerByFormId::iterator;
        using ReverseFormIdIterator = ContainerByFormId::reverse_iterator;
        using NameIterator          = ContainerByName::iterator;
        using ReverseNameIterator   = ContainerByName::reverse_iterator;

    private:
        Container   m_Container;
        UnusedArmor m_unusedArmor;
        // ContainerByFormId &m_containerByFormId = get<0>(m_Container);
        ContainerByName &m_containerByName = get<1>(m_Container);

    public:
        struct invalid_page_size : std::runtime_error
        {
            explicit invalid_page_size() : runtime_error("Invalid Page Size 0") {}
        };

        ArmorList()  = default;
        ~ArmorList() = default;

        void Insert(const ArmorWrap &armorWrap, bool isUsed = false)
        {
            FormIdIterator iter = m_Container.cend();
            if (armorWrap.armorPtr == nullptr)
            {
                return;
            }
            if (isUsed)
            {
                m_Container.insert(armorWrap);
            }
            else
            {
                m_Container.erase(armorWrap.armorPtr->GetFormID());
                m_unusedArmor.insert(armorWrap);
            }
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
        }

        void Insert(Armor *armor, bool isUsed = false)
        {
            Insert(ArmorWrap(armor), isUsed);
        }

        void RestoreUnusedArmors()
        {
            for (auto itBegin = m_unusedArmor.begin(); itBegin != m_unusedArmor.end();)
            {
                m_Container.insert(*itBegin);
                itBegin = m_unusedArmor.erase(itBegin);
            }
        }

        void Remove(Armor *armor)
        {
            if (armor == nullptr)
            {
                return;
            }
            m_unusedArmor.insert(armor);
            m_Container.erase(armor->GetFormID());
        }

        void Clear()
        {
            m_Container.clear();
            m_unusedArmor.clear();
        }

        constexpr auto IsEmpty() const -> bool
        {
            return m_Container.empty();
        }

        constexpr auto Size() const -> size_t
        {
            return m_Container.size();
        }

        auto GetAllArmorCount() const -> uint32_t
        {
            return m_unusedArmor.size() + m_Container.size();
        }

        /// Page Function

        template <typename Func>
        void for_each(Func &&func)
        {
            for (auto it = m_containerByName.begin(); it != m_containerByName.end(); ++it)
            {
                func(*it);
            }
        }

        template <typename Func>
        void for_each(size_t startPos, size_t endPos, Func &&func)
        {
            if (startPos > endPos)
            {
                reverse_for_each(startPos, endPos, func);
                return;
            }
            if (startPos >= m_containerByName.size())
            {
                return;
            }
            auto itBegin = m_containerByName.nth(startPos);
            if constexpr (std::is_invocable_v<Func &&, Armor *, size_t>)
            {
                for (auto &it = itBegin; startPos < endPos && it != m_containerByName.end(); ++startPos, ++it)
                {
                    func(it->armorPtr, startPos);
                }
            }
            else if constexpr (std::is_invocable_v<Func &&, Armor *>)
            {
                for (auto &it = itBegin; startPos < endPos && it != m_containerByName.end(); ++startPos, ++it)
                {
                    func(it->armorPtr);
                }
            }
        }

        template <typename Func>
        void reverse_for_each(size_t startPos, size_t endPos, Func &&func)
        {
            if (startPos > m_containerByName.size() || endPos > startPos)
            {
                return;
            }
            auto   itBegin = boost::make_reverse_iterator(m_containerByName.nth(startPos));
            auto   itEnd   = boost::make_reverse_iterator(m_containerByName.nth(endPos));
            size_t index   = 0;
            if constexpr (std::is_invocable_v<Func &&, Armor *, size_t>)
            {
                for (auto &it = itBegin; it != itEnd; ++it)
                {
                    func(it->armorPtr, index);
                    ++index;
                }
            }
            else if constexpr (std::is_invocable_v<Func &&, Armor *>)
            {
                for (auto &it = itBegin; it != itEnd; ++it)
                {
                    func(it->armorPtr);
                    ++index;
                }
            }
        }
    };
}