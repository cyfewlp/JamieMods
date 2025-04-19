#pragma once

namespace LIBC_NAMESPACE_DECL
{
    namespace util
    {
        constexpr auto reverse_range(size_t start, size_t end, size_t itemCount) -> std::pair<size_t, size_t>
        {
            start       = itemCount - start;
            auto length = end - start;
            end         = start < length ? 0 : start - length;
            return {start, end};
        }
    }
}