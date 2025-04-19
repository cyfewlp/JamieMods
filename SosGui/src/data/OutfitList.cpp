#include "data/OutfitList.h"
#include "common/config.h"
#include "data/id.h"
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    auto OutfitList::Rank(const OutfitId &id) const -> size_t
    {
        if (!HasOutfit(id))
        {
            throw unassociated_outfit_error();
        }
        if (m_onlyFavorite)
        {
            auto itById   = m_favorites.find(id);
            auto itByName = project<by_name>(m_favorites, itById);
            return get<by_name>(m_favorites).rank(itByName);
        }

        auto itById   = m_outfitById.find(id);
        auto itByName = project<by_name>(m_container, itById);
        return m_outfitByName.rank(itByName);
    }

    auto OutfitList::findByName(const std::string &outfitName) const -> OutfitId
    {
        auto itByName = m_outfitByName.find(outfitName);
        if (itByName == m_outfitByName.end())
        {
            return INVALID_ID;
        }
        return itByName->GetId();
    }

}