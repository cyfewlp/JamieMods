#pragma once

#include "common/config.h"
#include "data/BaseContainer.h"
#include "util/StringUtil.h"

#if !defined(NDEBUG)
#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#include <RE/B/BSCoreTypes.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/optional/optional.hpp>

namespace
LIBC_NAMESPACE_DECL
{
using namespace boost::multi_index;

class ArmorView : BaseContainer
{
    using Armor = RE::TESObjectARMO;

    struct by_FormId {};

    struct by_name {};

public:
    typedef BOOST_MULTI_INDEX_CONST_MEM_FUN(RE::TESForm, RE::FormID, GetFormID) KeyByFormId;
    typedef BOOST_MULTI_INDEX_CONST_MEM_FUN(RE::TESForm, const char *, GetName) KeyByName;

    struct ArmorViewIndex : indexed_by<ordered_unique<tag<by_FormId>, KeyByFormId>,
                                       ranked_non_unique<tag<by_name>, KeyByName, util::StringCompactor>> {};

    typedef multi_index_container<Armor *, ArmorViewIndex> Container;

    typedef index<Container, by_FormId>::type ContainerByFormId;
    typedef index<Container, by_name>::type ContainerByName;

private:
    Container m_container{};
    ContainerByFormId &m_indexByFormId = get<by_FormId>(m_container);
    ContainerByName &m_indexByName = get<by_name>(m_container);

public:
    ArmorView() = default;
    ~ArmorView() = default;

    void Insert(Armor *armor);

    auto Remove(Armor *armor) const -> bool;

    auto GetByNameRank(size_t rank) const -> std::optional<Armor *>;

    auto GetRank(const RE::FormID armorFormId) const -> size_t
    {
        auto itById = m_indexByFormId.find(armorFormId);
        return m_indexByName.rank(project<by_name>(m_container, itById));
    }

    void Clear()
    {
        m_container.clear();
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
        return m_container.size();
    }

    auto erase(Container::iterator where) -> Container::iterator
    {
        return m_container.erase(where);
    }

    auto begin() -> Container::iterator
    {
        return m_container.begin();
    }

    auto end() -> Container::iterator
    {
        return m_container.end();
    }

    /// Page Function

    template <typename Func>
    void for_each(Func &&func)
    {
        for (auto it = m_indexByName.begin(); it != m_indexByName.end(); ++it)
        {
            func(*it);
        }
    }

    template <typename Func>
    void for_each(bool ascend, size_t startPos, size_t endPos, Func &&func)
    {
        if (ascend)
        {
            for_each(startPos, endPos, func);
        }
        else
        {
            reverse_for_each(startPos, endPos, func);
        }
    }

    template <typename Func>
    void for_each(size_t startPos, size_t endPos, Func &&func)
    {
        for_each_on(get<by_name>(m_container), startPos, endPos, [&](const auto &armor, size_t index) {
            do_each(std::forward<Func>(func), armor, index);
        });
    }

    template <typename Func>
    void reverse_for_each(size_t startPos, size_t endPos, Func &&func)
    {
        reverse_for_each_on(get<by_name>(m_container), startPos, endPos, [&](const auto &armor, size_t index) {
            do_each(std::forward<Func>(func), armor, index);
        });
    }
};
}