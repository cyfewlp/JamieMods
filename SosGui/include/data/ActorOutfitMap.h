#pragma once

#include "data/id.h"
#include <boost/optional/optional.hpp>
#include <unordered_map>

namespace LIBC_NAMESPACE_DECL
{
    class ActorOutfitMap
    {
        using Container = std::unordered_map<RE::Actor *, OutfitId>;

        Container m_container{};

    public:
        void SetOutfit(RE::Actor *actor, OutfitId outfitId)
        {
            if (actor == nullptr)
            {
                return;
            }
            m_container[actor] = outfitId;
        }

        auto TryGetOutfitId(RE::Actor *actor) const -> boost::optional<OutfitId>
        {
            return m_container.contains(actor) ? boost::optional<OutfitId>(m_container.at(actor)) : boost::none;
        }

        auto GetOutfitId(RE::Actor *actor) const -> OutfitId
        {
            return m_container.contains(actor) ? m_container.at(actor) : INVALID_OUTFIT_ID;
        }

        auto HasActiveOutfit(RE::Actor *actor) const -> bool
        {
            return m_container.contains(actor);
        }

        auto IsActorOutfit(RE::Actor *actor, OutfitId outfitId) const -> bool
        {
            if (!m_container.contains(actor))
            {
                return false;
            }
            return m_container.at(actor) == outfitId;
        }
    };
}