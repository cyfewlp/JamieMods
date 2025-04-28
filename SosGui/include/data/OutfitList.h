#pragma once

#include "common/config.h"
#include "data/BaseContainer.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "util/StringUtil.h"

#if !defined(NDEBUG)
    #define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
    #define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/optional.hpp>
#include <boost/optional/detail/optional_reference_spec.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <cstdint>
#include <expected>
#include <stdexcept>
#include <string>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
using namespace boost::multi_index;

class OutfitList : BaseContainer
{
    static inline OutfitId g_NextOutfitId = 1;

public:
    struct by_Id
    {
    };

    struct by_name
    {
    };

    struct by_favorite
    {
    };

    struct favorite_name_key
        : composite_key<SosUiOutfit, BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, bool, IsFavorite),
                        BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, const std::string &, GetName)>
    {
    };

    using favorite_name_compare = composite_key_compare<std::greater<bool>, util::StringCompactor>;

    typedef BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, OutfitId, GetId) IdKey;
    typedef BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, const std::string &, GetName) NameKey;

    struct outfit_index : indexed_by<                                                          //
                              ordered_unique<tag<by_Id>, IdKey>,                               //
                              ranked_non_unique<tag<by_name>, NameKey, util::StringCompactor>, //
                              ranked_non_unique<tag<by_favorite>, favorite_name_key, favorite_name_compare>>
    {
    };

    typedef multi_index_container<SosUiOutfit, outfit_index> Container;

    using Armor         = RE::TESObjectARMO;
    using OutfitById    = index<Container, by_Id>::type;
    using OutfitByName  = index<Container, by_name>::type;
    using FavoriteIndex = index<Container, by_favorite>::type;

private:
    Container      m_container{};
    OutfitById    &m_outfitById    = get<by_Id>(m_container);
    OutfitByName  &m_outfitByName  = get<by_name>(m_container);
    FavoriteIndex &m_favoriteIndex = get<by_favorite>(m_container);

    struct unassociated_outfit_error : std::runtime_error
    {
        explicit unassociated_outfit_error()
            : std::runtime_error("WARNING: The specify outfit id is invalid or unassociate outfit. Try reopen or "
                                 "refresh outfit list")
        {
        }
    };

public:
    OutfitList()  = default;
    ~OutfitList() = default;

    OutfitList(const OutfitList &other)            = delete;
    OutfitList &operator=(const OutfitList &other) = delete;
    OutfitList(OutfitList &&other)                 = delete;
    OutfitList &operator=(OutfitList &&other)      = delete;

    //////////////////////////////////////////////////////////////////////////
    // Modifiers
    //////////////////////////////////////////////////////////////////////////

    template <typename String>
    constexpr void AddOutfit(String &&outfitName)
    {
        m_container.emplace(g_NextOutfitId, std::move(outfitName));
        ++g_NextOutfitId;
    }

    void AddOutfits(auto &&container)
    {
        for (const auto &outfit : container)
        {
            AddOutfit(outfit);
        }
    }

    [[nodiscard]] auto SetFavoriteOutfit(const OutfitId id, bool favorite)
        -> std::expected<void, unassociated_outfit_error>;

    [[nodiscard]] auto SetFavoriteOutfit(const std::string &outfitName, bool favorite)
        -> std::expected<void, unassociated_outfit_error>;

    auto GetOutfitByNameRank(const size_t rank) const -> boost::optional<const SosUiOutfit &>;

    auto GetOutfitById(const OutfitId id) const -> boost::optional<const SosUiOutfit &>;

    void RenameOutfit(const OutfitId id, const std::string &&newName);

    void AddArmor(const OutfitId id, Armor *armor);

    template <typename Container>
    void AddArmors(const OutfitId id, const Container &armors)
    {
        if (auto where = m_outfitById.find(id); where != m_outfitById.end())
        {
            m_outfitById.modify(where, [&](auto &outfit) {
                for (const auto &armor : armors)
                {
                    outfit.AddArmor(armor);
                }
            });
        }
    }

    void DeleteOutfit(const OutfitId id)
    {
        m_outfitById.erase(id);
    }

    void DeleteArmor(const OutfitId id, const Armor *armor)
    {
        if (auto where = m_outfitById.find(id); where != m_outfitById.end())
        {
            m_outfitById.modify(where, [&](auto &outfit) { outfit.RemoveArmor(armor); });
        }
    }

    void SetSlotPolicy(const OutfitId id, uint32_t slotPos, std::string &&policyString)
    {
        if (auto where = m_outfitById.find(id); where != m_outfitById.end())
        {
            m_outfitById.modify(where, [&](auto &outfit) { outfit.SetSlotPolicies(slotPos, std::move(policyString)); });
        }
    }

    void SetAllSlotPolicies(const OutfitId id, std::vector<std::string> slotPolicies)
    {
        if (auto where = m_outfitById.find(id); where != m_outfitById.end())
        {
            m_outfitById.modify(where, [&](auto &outfit) {
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

    //////////////////////////////////////////////////////////////////////////
    // Query
    //////////////////////////////////////////////////////////////////////////

    auto GetNameRank(const OutfitId &id) const -> size_t;

    auto TryFindIdByName(const std::string &outfitName) const -> boost::optional<OutfitId>;
    auto findIdByName(const std::string &outfitName) const -> OutfitId;

    auto HasOutfit(const OutfitId &id) const -> bool
    {
        return m_outfitById.contains(id);
    }

    [[nodiscard]] constexpr auto empty() const -> bool
    {
        return m_container.empty();
    }

    [[nodiscard]] constexpr auto size() const -> size_t
    {
        return m_container.size();
    }

    [[nodiscard]] constexpr auto FavoriteRankedIndex() const -> const FavoriteIndex &
    {
        return m_favoriteIndex;
    }

    [[nodiscard]] constexpr auto NameRankedIndex() const -> const OutfitByName &
    {
        return m_outfitByName;
    }

    [[nodiscard]] auto ProjectToNameIterator(FavoriteIndex::iterator &iter) const -> OutfitByName::iterator
    {
        return project<by_name>(m_container, iter);
    }

    //////////////////////////////////////////////////////////////////////////
    // Iterator
    //////////////////////////////////////////////////////////////////////////

    template <typename Func>
    void for_each(Func &&func) const
    {
        using namespace boost::adaptors;
        for (const auto &element : m_outfitByName | indexed())
        {
            func(element.value(), element.index());
        }
    }

    template <typename Func>
    void for_each(bool ascend, size_t startPos, size_t endPos, Func &&func) const
    {
        if (empty())
        {
            return;
        }
        if (ascend)
        {
            for_each(startPos, endPos, std::forward<Func>(func));
        }
        else
        {
            reverse_for_each(startPos, endPos, std::forward<Func>(func));
        }
    }

    template <typename Func>
    void for_each(size_t startPos, size_t endPos, Func &&func) const
    {
        for_each_on(m_outfitByName, startPos, endPos, [&](const auto &outfit, size_t index) {
            BaseContainer::do_each(std::forward<Func>(func), outfit, index);
        });
    }

    template <typename Func>
    void reverse_for_each(size_t startPos, size_t endPos, Func &&func) const
    {
        reverse_for_each_on(m_outfitByName, startPos, endPos, [&](const auto &outfit, size_t index) {
            BaseContainer::do_each(std::forward<Func>(func), outfit, index);
        });
    }
};

}