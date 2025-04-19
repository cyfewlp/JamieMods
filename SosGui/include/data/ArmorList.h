#pragma once

#include "common/config.h"
#include "common/log.h"
#include "util/StringUtil.h"

#if !defined(NDEBUG)
    #define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
    #define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#include <boost/assert.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    using namespace boost::multi_index;

    class ArmorList
    {
        using Armor = RE::TESObjectARMO;

        struct by_FormId
        {
        };

        struct by_name
        {
        };

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
                random_access<>,
                ordered_unique<tag<by_FormId>, BOOST_MULTI_INDEX_CONST_MEM_FUN(ArmorWrap, RE::FormID, GetFormID)>,
                ranked_non_unique<tag<by_name>, BOOST_MULTI_INDEX_CONST_MEM_FUN(ArmorWrap, const char *, GetName),
                                  util::StringCompactor>>>
            Container;

        typedef std::unordered_set<ArmorWrap, ArmorWrap::Hash> UnusedArmor;
        typedef index<Container, by_FormId>::type              ContainerByFormId;
        typedef index<Container, by_name>::type                ContainerByName;

        using FormIdIterator        = ContainerByFormId::iterator;
        using ReverseFormIdIterator = ContainerByFormId::reverse_iterator;
        using NameIterator          = ContainerByName::iterator;
        using ReverseNameIterator   = ContainerByName::reverse_iterator;

    private:
        Container          m_container;
        UnusedArmor        m_unusedArmor;
        ContainerByFormId &m_containerByFormId = get<by_FormId>(m_container);
        ContainerByName   &m_containerByName   = get<by_name>(m_container);

    public:
        struct invalid_page_size : std::runtime_error
        {
            explicit invalid_page_size() : runtime_error("Invalid Page Size 0") {}
        };

        ArmorList()  = default;
        ~ArmorList() = default;

        void Insert(const ArmorWrap &armorWrap, bool isUsed = false)
        {
            if (armorWrap.armorPtr == nullptr)
            {
                return;
            }
            if (isUsed)
            {
                m_container.insert(m_container.end(), armorWrap);
            }
            else
            {
                m_containerByFormId.erase(armorWrap.GetFormID());
                m_unusedArmor.insert(armorWrap);
            }
        }

        void Insert(const std::vector<ArmorWrap> &armorList, bool isUsed = false)
        {
            if (isUsed)
            {
                for (const auto &armorWrap : armorList)
                {
                    m_container.insert(m_container.end(), armorWrap);
                    m_unusedArmor.erase(armorWrap);
                }
            }
            else
            {
                for (const auto &armorWrap : armorList)
                {
                    m_containerByFormId.erase(armorWrap.GetFormID());
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
                m_container.insert(m_container.end(), *itBegin);
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
            m_containerByFormId.erase(armor->GetFormID());
        }

        void Clear()
        {
            m_container.clear();
            m_unusedArmor.clear();
        }

        constexpr auto IsEmpty() const -> bool
        {
            return m_container.empty();
        }

        constexpr auto Size() const -> size_t
        {
            return m_container.size();
        }

        auto GetAllArmorCount() const -> uint32_t
        {
            return m_unusedArmor.size() + m_container.size();
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
        void for_each(bool isReverse, size_t startPos, size_t endPos, Func &&func)
        {
            if (isReverse)
            {
                reverse_for_each(startPos, endPos, func);
            }
            else
            {
                for_each(startPos, endPos, func);
            }
        }

        template <typename Func>
        void for_each(size_t startPos, size_t endPos, Func &&func)
        {
            BOOST_ASSERT_MSG(startPos <= endPos, "Invalid range specify: start-pos can't > endPos");
            if (startPos >= m_containerByName.size())
            {
                return;
            }
            using namespace boost::adaptors;
            if constexpr (std::is_invocable_v<Func &&, Armor *, size_t>)
            {
                for (const auto &element : m_container | sliced(startPos, endPos) | indexed(startPos))
                {
                    func(element.value().armorPtr, element.index());
                }
            }
            else if constexpr (std::is_invocable_v<Func &&, Armor *>)
            {
                for (const auto &element : m_container | sliced(startPos, endPos))
                {
                    func(element.armorPtr);
                }
            }
        }

        template <typename Func>
        void reverse_for_each(size_t startPos, size_t endPos, Func &&func)
        {
            BOOST_ASSERT_MSG(startPos <= endPos, "Invalid range specify: start-pos can't > endPos");
            if (startPos >= m_containerByName.size())
            {
                return;
            }
            using namespace boost::adaptors;
            if constexpr (std::is_invocable_v<Func &&, Armor *, size_t>)
            {
                for (const auto &element : m_container | reversed | sliced(startPos, endPos) | indexed(startPos))
                {
                    func(element.value().armorPtr, element.index());
                }
            }
            else if constexpr (std::is_invocable_v<Func &&, Armor *>)
            {
                for (const auto &element : m_container | reversed | sliced(startPos, endPos))
                {
                    func(element.armorPtr);
                }
            }
        }
    };
}