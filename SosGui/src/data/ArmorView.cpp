#include "data/ArmorView.h"
#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
    void ArmorView::Insert(Armor *armor)
    {
        if (armor == nullptr)
        {
            return;
        }
        m_container.insert(m_container.end(), armor);
    }
}