#pragma once

#include "common/config.h"
#include "data/BaseContainer.h"
#include "data/SosUiOutfit.h"
#include "data/id.h"
#include "util/StringUtil.h"
#include "util/utils.h"

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
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
    using namespace boost::multi_index;

    class OutfitList : BaseContainer
    {
        static inline OutfitId g_NextOutfitId = 1;

        struct by_Id
        {
        };

        struct by_name
        {
        };

        struct favorite_by_name
        {
        };

        struct favorite_outfit
        {
            OutfitId    id = INVALID_ID;
            std::string name;

            constexpr auto GetId() const -> OutfitId
            {
                return id;
            }

            constexpr auto GetName() const -> const std::string &
            {
                return name;
            }
        };

        struct favorite_name_key
            : composite_key<SosUiOutfit, BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, bool, IsFavorite),
                            BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, const std::string &, GetName)>
        {
        };

        using favorite_name_compare = composite_key_compare<std::less<bool>, util::StringCompactor>;

        typedef BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, OutfitId, GetId) IdKey;
        typedef BOOST_MULTI_INDEX_CONST_MEM_FUN(SosUiOutfit, const std::string &, GetName) NameKey;

        struct outfit_index : indexed_by<ordered_unique<tag<by_Id>, IdKey>,
                                         ranked_non_unique<tag<by_name>, NameKey, util::StringCompactor>>
        {
        };

        typedef boost::multi_index_container<SosUiOutfit, outfit_index>         Container;
        typedef boost::multi_index_container<const SosUiOutfit *, outfit_index> FavoriteContainer;

        using Armor             = RE::TESObjectARMO;
        using OutfitById        = index<Container, by_Id>::type;
        using OutfitByName      = index<Container, by_name>::type;
        using IdIterator        = OutfitById::iterator;
        using ReverseIdIterator = OutfitById::reverse_iterator;

        Container         m_container{};
        FavoriteContainer m_favorites;
        OutfitById       &m_outfitById   = get<by_Id>(m_container);
        OutfitByName     &m_outfitByName = get<by_name>(m_container);
        bool              m_onlyFavorite = false;

        struct unassociated_outfit_error : std::runtime_error
        {
            explicit unassociated_outfit_error()
                : std::runtime_error("The provide outfit id is unassociated any outfit.")
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

        void SetFavoriteOutfit(const OutfitId id, bool favorite) noexcept(false)
        {
            auto outfitIt = m_outfitById.find(id);
            if (outfitIt == m_outfitById.end())
            {
                throw unassociated_outfit_error{};
            }
            m_favorites.erase(id);
            m_favorites.insert(&*outfitIt);
            m_outfitById.modify(outfitIt, [&](auto &outfit) {
                outfit.SetFavorite(favorite);
            });
        }

        auto GetOutfit(const OutfitId id) const -> boost::optional<const SosUiOutfit &>
        {
            boost::optional<const SosUiOutfit &> opt;

            if (auto foundId = m_outfitById.find(id); foundId != m_outfitById.end())
            {
                opt = *foundId;
            }
            return opt;
        }

        void RenameOutfit(const OutfitId id, const std::string &&newName)
        {
            if (auto where = m_outfitById.find(id); where != m_outfitById.end())
            {
                m_outfitById.modify(where, [&](auto &outfit) {
                    outfit.SetName(newName);
                });
            }
        }

        void AddArmor(const OutfitId id, Armor *armor)
        {
            if (auto where = m_outfitById.find(id); where != m_outfitById.end())
            {
                m_outfitById.modify(where, [&](auto &outfit) {
                    outfit.AddArmor(armor);
                });
            }
        }

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
                m_outfitById.modify(where, [&](auto &outfit) {
                    outfit.RemoveArmor(armor);
                });
            }
        }

        void SetSlotPolicy(const OutfitId id, uint32_t slotPos, std::string &&policyString)
        {
            if (auto where = m_outfitById.find(id); where != m_outfitById.end())
            {
                m_outfitById.modify(where, [&](auto &outfit) {
                    outfit.SetSlotPolicies(slotPos, std::move(policyString));
                });
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

        // only iterate favorite outfits when call for_each
        void OnlyFavoriteOutfits(bool yes)
        {
            m_onlyFavorite = yes;
        }

        constexpr void clear()
        {
            m_container.clear();
        }

        //////////////////////////////////////////////////////////////////////////
        // Query
        //////////////////////////////////////////////////////////////////////////

        auto Rank(const OutfitId &id) const -> size_t;

        auto findByName(const std::string &outfitName) const -> OutfitId;

        auto HasOutfit(const OutfitId &id) const -> bool
        {
            return m_outfitById.contains(id);
        }

        [[nodiscard]] constexpr auto empty() const -> bool
        {
            return m_onlyFavorite ? m_favorites.empty() : m_container.empty();
        }

        [[nodiscard]] constexpr auto size() -> size_t
        {
            return m_onlyFavorite ? m_favorites.size() : m_container.size();
        }

        //////////////////////////////////////////////////////////////////////////
        // Iterator
        //////////////////////////////////////////////////////////////////////////

        template <typename Func>
        void for_each(Func &&func)
        {
            using namespace boost::adaptors;
            if (m_onlyFavorite)
            {
                for (const auto &element : get<by_name>(m_favorites) | indexed())
                {
                    func(*element.value(), element.index());
                }
            }
            else
            {
                for (const auto &element : m_outfitByName | indexed())
                {
                    func(element.value(), element.index());
                }
            }
        }

        template <typename Func>
        void for_each(bool ascend, size_t startPos, size_t endPos, Func &&func)
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
        void for_each(size_t startPos, size_t endPos, Func &&func)
        {
            if (m_onlyFavorite)
            {
                for_each_on(get<by_name>(m_favorites), startPos, endPos, [&](const auto &outfit, size_t index) {
                    BaseContainer::do_each(std::forward<Func>(func), *outfit, index);
                });
            }
            else
            {
                for_each_on(m_outfitByName, startPos, endPos, [&](const auto &outfit, size_t index) {
                    BaseContainer::do_each(std::forward<Func>(func), outfit, index);
                });
            }
        }

        template <typename Func>
        void reverse_for_each(size_t startPos, size_t endPos, Func &&func)
        {
            if (m_onlyFavorite)
            {
                reverse_for_each_on(get<by_name>(m_favorites), startPos, endPos, [&](const auto &outfit, size_t index) {
                    BaseContainer::do_each(std::forward<Func>(func), *outfit, index);
                });
            }
            else
            {
                reverse_for_each_on(m_outfitByName, startPos, endPos, [&](const auto &outfit, size_t index) {
                    BaseContainer::do_each(std::forward<Func>(func), outfit, index);
                });
            }
        }
    };
}