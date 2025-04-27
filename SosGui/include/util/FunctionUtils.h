#pragma once

#include "common/config.h"
#include "data/OutfitList.h"

namespace LIBC_NAMESPACE_DECL
{
    struct GetOutfitFromId
    {
        OutfitList &outfitList;

        explicit GetOutfitFromId(OutfitList &outfitList) : outfitList(outfitList) {}

        auto operator()(auto outfitId) -> boost::optional<const SosUiOutfit &>
        {
            return outfitList.GetOutfitById(outfitId);
        }
    };
}