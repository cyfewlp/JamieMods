#include "util/ImGuiUtil.h"
#include "common/config.h"

namespace LIBC_NAMESPACE_DECL
{
    void ImGuiUtil::may_update_table_sort_dir(bool &ascend)
    {
        if (auto *sortSpecs = ImGui::TableGetSortSpecs(); sortSpecs != nullptr)
        {
            if (sortSpecs->SpecsDirty)
            {
                const auto direction  = sortSpecs->Specs[0].SortDirection;
                ascend                = direction == ImGuiSortDirection_Ascending;
                sortSpecs->SpecsDirty = false;
            }
        }
    }
}