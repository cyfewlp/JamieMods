#include "data/ArmorContainer.h"
#include "common/config.h"

namespace
LIBC_NAMESPACE_DECL
{
void ArmorContainer::Insert(Armor *armor)
{
    if (armor == nullptr)
    {
        return;
    }
    const auto it = std::ranges::lower_bound(m_container, armor, NameComparator());
    m_container.insert(it, armor);
}

auto ArmorContainer::GetRank(const char *armorName, RE::FormID formId) const -> size_t
{
    if (armorName == nullptr)
    {
        return m_container.size();
    }
    auto foundIt = std::lower_bound(
        m_container.begin(), m_container.end(),
        armorName,
        [](const Armor *armor, const char *searchName) {
            return util::StringCompactor()(armor->GetName(), searchName);
        }
        );

    if (foundIt == m_container.end() || (*foundIt)->formID == formId)
    {
        return std::distance(m_container.begin(), foundIt);
    }
    while (++foundIt != m_container.end())
    {
        if ((*foundIt)->GetName() != armorName)
        {
            break;
        }
        if (formId == (*foundIt)->GetFormID())
        {
            break;
        }
    }

    return std::distance(m_container.begin(), foundIt);
}

}