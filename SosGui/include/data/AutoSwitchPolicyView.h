#pragma once

#include "common/config.h"
#include "data/BaseContainer.h"
#include "data/id.h"

#if !defined(NDEBUG)
    #define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
    #define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#include <RE/A/Actor.h>
#include <RE/B/BSCoreTypes.h>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/detail/ord_index_impl.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/none.hpp>
#include <boost/optional/optional.hpp>
#include <boost/tuple/detail/tuple_basic.hpp>
#include <cstdint>
#include <format>
#include <stdexcept>

namespace LIBC_NAMESPACE_DECL
{

using namespace boost::multi_index;

class AutoSwitchPolicyView : BaseContainer
{
    using Actor = RE::Actor;

    struct by_FormId
    {
    };

    struct by_name
    {
    };

public:
    enum class Policy : uint32_t
    {
        Combat  = 12,
        World   = 0,
        Town    = 1,
        Dungeon = 2,
        City    = 9,

        WorldSnowy   = 3,
        TownSnowy    = 4,
        DungeonSnowy = 5,
        CitySnowy    = 10,

        WorldRainy   = 6,
        TownRainy    = 7,
        DungeonRainy = 8,
        CityRainy    = 11,
        Count        = 13,
        None         = 14
    };

    struct PolicyEntry
    {
        RE::FormID actorId;
        Policy     policy;
        OutfitId   outfitId;

        explicit PolicyEntry(const RE::FormID actorId, uint32_t policyId, const OutfitId id)
            : actorId(actorId), policy(static_cast<Policy>(policyId)), outfitId(id)
        {
        }

        explicit PolicyEntry(const RE::FormID actorId, const Policy policy, const OutfitId id)
            : actorId(actorId), policy(policy), outfitId(id)
        {
        }
    };

    typedef BOOST_MULTI_INDEX_MEMBER(PolicyEntry, RE::FormID, actorId) KeyByFormId;
    typedef BOOST_MULTI_INDEX_MEMBER(PolicyEntry, Policy, policy) KeyByPolicy;

    struct policy_key : composite_key<PolicyEntry, KeyByFormId, KeyByPolicy>
    {
    };

    typedef multi_index_container<PolicyEntry, indexed_by<ordered_unique<policy_key>>> Container;

    typedef nth_index<Container, 0>::type ContainerByPolicy;

private:
    Container m_container{};

public:
    AutoSwitchPolicyView()  = default;
    ~AutoSwitchPolicyView() = default;

    template <typename Entry>
    void Insert(Entry &&entry)
    {
        m_container.insert(std::forward<Entry>(entry));
    }

    void emplace(RE::FormID actorId, uint32_t policyId, OutfitId outfitId)
    {
        auto policy = static_cast<Policy>(policyId);
        if (policy >= Policy::Count)
        {
            throw std::invalid_argument(std::format("Invalid policy id: {}", policyId));
        }
        m_container.emplace(actorId, policy, outfitId);
    }

    void Clear()
    {
        m_container.clear();
    }

    auto TryFind(const RE::FormID formId, uint32_t uintPolicy) const -> boost::optional<Container::iterator>
    {
        if (const auto policy = static_cast<Policy>(uintPolicy); policy < Policy::None)
        {
            auto it = m_container.find(boost::make_tuple(formId, policy));
            return boost::make_optional(it != m_container.end(), it);
        }
        return boost::none;
    }

    auto find(const RE::FormID formId, uint32_t uintPolicy) const -> Container::iterator
    {
        const auto policy = static_cast<Policy>(uintPolicy);
        if (policy >= Policy::None)
        {
            return m_container.end();
        }
        return m_container.find(boost::make_tuple(formId, policy));
    }

    constexpr auto IsEmpty() const -> bool
    {
        return m_container.empty();
    }

    constexpr auto Size() const -> size_t
    {
        return m_container.size();
    }

    template <typename Entry>
    auto erase(Entry &&entry) -> bool
    {
        return m_container.erase(std::forward<Entry>(entry)) > 0;
    }

    auto erase(const RE::FormID actorId) -> void
    {
        ContainerByPolicy::iterator it0, it1;
        boost::tie(it0, it1) = m_container.equal_range(boost::make_tuple(actorId));
        while (it0 != it1)
        {
            it0 = m_container.erase(it0);
        }
    }

    auto erase(const RE::FormID actorId, uint32_t policyId) -> void
    {
        const auto policy = static_cast<Policy>(policyId);
        if (policy >= Policy::Count)
        {
            throw std::invalid_argument(std::format("Invalid policy id: {}", policyId));
        }
        ContainerByPolicy::iterator it0, it1;
        boost::tie(it0, it1) = m_container.equal_range(boost::make_tuple(actorId, policy));
        while (it0 != it1)
        {
            it0 = m_container.erase(it0);
        }
    }

    auto erase(const Container::iterator &where) -> Container::iterator
    {
        return m_container.erase(where);
    }

    auto begin() -> Container::iterator
    {
        return m_container.begin();
    }

    auto begin() const -> Container::const_iterator
    {
        return m_container.begin();
    }

    auto end() -> Container::iterator
    {
        return m_container.end();
    }

    auto end() const -> Container::const_iterator
    {
        return m_container.end();
    }

    template <typename Func>
    void for_each(const RE::FormID actorId, Func &&func)
    {
        ContainerByPolicy::iterator it0, it1;
        boost::tie(it0, it1) = m_container.equal_range(boost::make_tuple(actorId));
        for (; it0 != it1; ++it0)
        {
            func(*it0);
        }
    }
};
}
