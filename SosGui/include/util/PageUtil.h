#pragma once

namespace LIBC_NAMESPACE_DECL
{
    namespace util
    {
        class PageUtil
        {
            static constexpr uint8_t DEFAULT_PAGE_SIZE = 20;

            uint16_t pageSize  = DEFAULT_PAGE_SIZE;
            uint16_t pageCount = 0;
            uint16_t pageIndex = 0;
            size_t   itemCount = 0;
            bool     ascend    = true;

        public:
            PageUtil()  = default;
            ~PageUtil() = default;

            PageUtil(const PageUtil &other)            = delete;
            PageUtil &operator=(const PageUtil &other) = delete;
            PageUtil(PageUtil &&other)                 = delete;
            PageUtil &operator=(PageUtil &&other)      = delete;

            constexpr auto HasPrevPage() const -> bool
            {
                return pageCount > 1 && pageIndex > 0;
            }

            constexpr auto HasNextPage() const -> bool
            {
                return pageCount > 1 && pageIndex < pageCount - 1;
            }

            void PrevPage()
            {
                pageIndex--;
            }

            void NextPage()
            {
                pageIndex++;
            }

            void SetAscendSort(bool ascend)
            {
                this->ascend = ascend;
            }

            constexpr auto GetPageIndex() const -> uint16_t
            {
                return pageIndex + 1;
            }

            constexpr auto GetPageCount() const -> uint16_t
            {
                return pageCount;
            }

            constexpr auto SetPageSize(uint16_t pageSize)
            {
                this->pageSize = pageSize;
                updatePageInfo(itemCount);
            }

            constexpr auto GetPageSize() const -> uint16_t
            {
                return pageSize;
            }

            [[nodiscard]] auto PageRange() const -> std::pair<size_t, size_t>
            {
                size_t start = pageIndex * pageSize;
                size_t end   = start + pageSize;
                if (ascend)
                {
                    return {start, end};
                }
                start = itemCount - start;
                end   = start < pageSize ? 0 : start - pageSize;
                return {start, end};
            }

            void SetItemCount(size_t itemCount)
            {
                static size_t lastItemCount = 0;
                if (lastItemCount != itemCount)
                {
                    lastItemCount = itemCount;
                    updatePageInfo(itemCount);
                }
            }

            void updatePageInfo(size_t itemCount);
        };

        void RenderPageWidgets(PageUtil &page);
    }
}