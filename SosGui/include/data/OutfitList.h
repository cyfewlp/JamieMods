#pragma once

#include "common/config.h"
#include "data/SosUiOutfit.h"
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
#include <boost/optional.hpp>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    using namespace boost::multi_index;

    class OutfitList
    {
        typedef boost::multi_index_container<
            SosUiOutfit,
            indexed_by< //
                ordered_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, SosUiOutfit::OutfitId, GetId)>,
                ranked_non_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, const std::string &, GetName),
                                  util::StringCompactor>>>
            Container;

        using Armor               = RE::TESObjectARMO;
        using ContainerById       = nth_index<Container, 0>::type;
        using ContainerByName     = nth_index<Container, 1>::type;
        using IdIterator          = ContainerById::iterator;
        using ReverseIdIterator   = ContainerById::reverse_iterator;
        using NameIterator        = ContainerByName::iterator;
        using ReverseNameIterator = ContainerByName::reverse_iterator;

        Container        m_container{};
        ContainerById   &m_containerById   = get<0>(m_container);
        ContainerByName &m_containerByName = get<1>(m_container);

    public:
        OutfitList()  = default;
        ~OutfitList() = default;

        OutfitList(const OutfitList &other)            = delete;
        OutfitList &operator=(const OutfitList &other) = delete;
        OutfitList(OutfitList &&other)                 = delete;
        OutfitList &operator=(OutfitList &&other)      = delete;

        template <typename String>
        constexpr void AddOutfit(String &&outfitName)
        {
            static SosUiOutfit::OutfitId g_NextOutfitId = 1;
            m_container.emplace_hint(m_containerById.end(), g_NextOutfitId, std::move(outfitName));
            ++g_NextOutfitId;
        }

        void AddOutfits(auto &&container)
        {
            for (const auto &outfit : container)
            {
                AddOutfit(outfit);
            }
        }

        auto GetOutfit(const SosUiOutfit::OutfitId id) const -> boost::optional<const SosUiOutfit &>
        {
            boost::optional<const SosUiOutfit &> opt;

            if (auto foundId = m_containerById.find(id); foundId != m_containerById.end())
            {
                opt = *foundId;
            }
            return opt;
        }

        void RenameOutfit(const SosUiOutfit::OutfitId id, const std::string &&newName)
        {
            if (auto where = m_containerById.find(id); where != m_containerById.end())
            {
                m_containerById.modify(where, [&](auto &outfit) {
                    outfit.SetName(newName);
                });
            }
        }

        void AddArmor(const SosUiOutfit::OutfitId id, Armor *armor)
        {
            if (auto where = m_containerById.find(id); where != m_containerById.end())
            {
                m_containerById.modify(where, [&](auto &outfit) {
                    outfit.AddArmor(armor);
                });
            }
        }

        template <typename Container>
        void AddArmors(const SosUiOutfit::OutfitId id, const Container &armors)
        {
            if (auto where = m_containerById.find(id); where != m_containerById.end())
            {
                m_containerById.modify(where, [&](auto &outfit) {
                    for (const auto &armor : armors)
                    {
                        outfit.AddArmor(armor);
                    }
                });
            }
        }

        void DeleteOutfit(const SosUiOutfit::OutfitId id)
        {
            m_containerById.erase(id);
        }

        void DeleteArmor(const SosUiOutfit::OutfitId id, const Armor *armor)
        {
            if (auto where = m_containerById.find(id); where != m_containerById.end())
            {
                m_containerById.modify(where, [&](auto &outfit) {
                    outfit.RemoveArmor(armor);
                });
            }
        }

        void SetSlotPolicy(const SosUiOutfit::OutfitId id, uint32_t slotPos, std::string &&policyString)
        {
            if (auto where = m_containerById.find(id); where != m_containerById.end())
            {
                m_containerById.modify(where, [&](auto &outfit) {
                    outfit.SetSlotPolicies(slotPos, std::move(policyString));
                });
            }
        }

        void SetAllSlotPolicies(const SosUiOutfit::OutfitId id, std::vector<std::string> slotPolicies)
        {
            if (auto where = m_containerById.find(id); where != m_containerById.end())
            {
                m_containerById.modify(where, [&](auto &outfit) {
                    for (uint32_t slotPos = 0; slotPos < SosUiOutfit::SLOT_COUNT; ++slotPos)
                    {
                        outfit.SetSlotPolicies(slotPos, slotPolicies[slotPos]);
                    }
                });
            }
        }

        constexpr void clear()
        {
            m_container.clear();
        }

        auto HasOutfit(SosUiOutfit::OutfitId &id) const -> bool
        {
            return m_container.contains(id);
        }

        [[nodiscard]] constexpr auto empty() -> bool
        {
            return m_container.empty();
        }

        [[nodiscard]] constexpr auto size() -> size_t
        {
            return m_container.size();
        }

        template <typename Func>
        void for_each(Func &&func)
        {
            size_t index = 0;
            for (auto it = m_containerByName.begin(); it != m_containerByName.end(); ++index, ++it)
            {
                func(*it, index);
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
            if constexpr (std::is_invocable_v<Func &&, const SosUiOutfit &, size_t>)
            {
                for (auto &it = itBegin; startPos < endPos && it != m_containerByName.end(); ++startPos, ++it)
                {
                    func(*it, startPos);
                }
            }
            else if constexpr (std::is_invocable_v<Func &&, const SosUiOutfit &>)
            {
                for (auto &it = itBegin; startPos < endPos && it != m_containerByName.end(); ++startPos, ++it)
                {
                    func(*it);
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
            auto itBegin = boost::make_reverse_iterator(m_containerByName.nth(startPos));
            auto itEnd = boost::make_reverse_iterator(m_containerByName.nth(endPos));
            size_t index = 0;
            if constexpr (std::is_invocable_v<Func &&, const SosUiOutfit &, size_t>)
            {
                for (auto &it = itBegin;it != itEnd;++it)
                {
                    func(*it, index);
                    ++index;
                }
            }
            else if constexpr (std::is_invocable_v<Func &&, const SosUiOutfit &>)
            {
                for (auto &it = itBegin; it != itEnd;++it)
                {
                    func(*it);
                    ++index;
                }
            }
        }
    };

}