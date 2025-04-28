#include "data/ArmorContainer.h"
#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
    void ArmorContainer::Insert(Armor *armor)
    {
        if (armor == nullptr)
        {
            return;
        }
        m_container.insert(m_container.end(), armor);
    }

    auto ArmorContainer::Remove(Armor *armor) const -> bool {
        if (armor == nullptr)
        {
            return false;
        }
        return m_indexByFormId.erase(armor->GetFormID()) > 0;
    }

    auto ArmorContainer::GetByNameRank(size_t rank) const -> std::optional<Armor *> {
        if (const auto foundId = m_indexByName.nth(rank); foundId != m_indexByName.end())
        {
            return *foundId;
        }
        return std::nullopt;
    }
}