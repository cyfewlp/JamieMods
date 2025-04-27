#include "util/PageUtil.h"
#include "common/config.h"
#include "imgui.h"
#include "util/ImGuiUtil.h"
#include <cstdint>
#include <format>
#include <utility>

namespace LIBC_NAMESPACE_DECL
{

void util::PageUtil::SetPageIndex(uint16_t pageIndex)
{
    if (pageIndex > pageCount)
    {
        throw std::invalid_argument("invalid page idnex.");
    }
    this->pageIndex = pageIndex;
    updatePageInfo(itemCount);
}

[[nodiscard]]
auto util::PageUtil::PageRange() const -> std::pair<size_t, size_t>
{
    size_t start = pageIndex * pageSize;
    size_t end   = start + pageSize;
    return {start, end};
}

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
    int   currentPage = page.GetPageIndex();
    float padding     = ImGui::GetStyle().FramePadding.x * 2.0f;
    float width       = ImGui::CalcTextSize(std::format("{}", currentPage + 1).c_str()).x + padding;
    ImGui::SetNextItemWidth(width);

    if (ImGui::DragInt("##CurrentPage", &currentPage, 1, 1, page.GetPageCount(), "%d", ImGuiSliderFlags_ClampOnInput))
    {
        page.SetPageIndex(currentPage);
    }

    ImGui::SameLine();
    ImGui::Text("/%d", page.GetPageCount());

    ImGui::SameLine();
    ImGui::BeginDisabled(!page.HasNextPage());
    if (ImGuiUtil::Button("$SosGui_Table_NextPage"))
    {
        page.NextPage();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    int curPageSize = page.GetPageSize();
    ImGui::SetNextItemWidth(-1.0F);
    if (ImGuiUtil::SliderInt("$SosGui_Table_PageSize", &curPageSize, 1, 500))
    {
        page.SetPageSize(static_cast<uint16_t>(curPageSize));
    }
}

}
