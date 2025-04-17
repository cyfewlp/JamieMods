#include "util/PageUtil.h"
#include "common/config.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"
#include <cstdint>

namespace LIBC_NAMESPACE_DECL
{
    void util::PageUtil::updatePageInfo(size_t itemCount)
    {
        this->itemCount = itemCount;
        pageCount       = itemCount / pageSize;
        if (pageCount * pageSize < itemCount)
        {
            pageCount += 1;
        }
        if (pageIndex >= pageCount)
        {
            pageIndex = std::max(0, pageCount - 1);
        }
    }

    void util::RenderPageWidgets(PageUtil &page)
    {
        ImGui::BeginDisabled(!page.HasPrevPage());
        if (ImGuiUtil::Button("$SosGui_Table_PrevPage"))
        {
            page.PrevPage();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Text("%d/%d", page.GetPageIndex(), page.GetPageCount());

        ImGui::SameLine();
        ImGui::BeginDisabled(!page.HasNextPage());
        if (ImGuiUtil::Button("$SosGui_Table_NextPage"))
        {
            page.NextPage();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        int curPageSize = page.GetPageSize();
        if (ImGuiUtil::SliderInt("$SosGui_Table_PageSize", &curPageSize, 1, 500))
        {
            page.SetPageSize(static_cast<uint16_t>(curPageSize));
        }
    }

}
