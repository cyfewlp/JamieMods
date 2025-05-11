#pragma once

#include "common/config.h"
#include "data/BaseContainer.h"
#include "util/StringUtil.h"

#include <RE/B/BSCoreTypes.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESObjectARMO.h>

namespace
LIBC_NAMESPACE_DECL
{
class ArmorContainer : BaseContainer
{
    using Armor = RE::TESObjectARMO;

    struct NameComparator
    {
        bool operator()(const Armor *lhs, const Armor *rhs) const
        {
            return util::StringCompactor()(lhs->GetName(), rhs->GetName());
        }
    };

    std::vector<Armor *> m_container;

public:
    ArmorContainer()  = default;
    ~ArmorContainer() = default;

    void Insert(Armor *armor);


    /**
     * The formId be used ensure the expected armor
     * because some armor may have the same name.
     */
    auto GetRank(const char *armorName, RE::FormID formId) const -> size_t;

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
};
}