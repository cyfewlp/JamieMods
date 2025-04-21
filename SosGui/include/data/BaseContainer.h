#pragma once

#include "util/utils.h"

#include <boost/iterator/reverse_iterator.hpp>
#include <stdexcept>
#include <type_traits>

namespace LIBC_NAMESPACE_DECL
{
    class BaseContainer
    {
    protected:
        template <typename RankedIndex>
        static constexpr void validate_range(const RankedIndex &index, size_t startPos, size_t endPos)
        {
            if (startPos >= index.size())
            {
                throw std::out_of_range(std::format("Invalid startPos: {} out of range {}", startPos, index.size()));
            }
            if (startPos > endPos)
            {
                throw std::invalid_argument(std::format("startPos {} can't greater endPos {}", startPos, endPos));
            }
        }

        template <typename RankedIndex, typename Func>
        static void for_each_on(const RankedIndex &index, size_t startPos, size_t endPos, Func &&func)
        {
            validate_range(index, startPos, endPos);
            for (auto itBegin = index.nth(startPos); startPos < endPos && itBegin != index.end(); ++startPos, ++itBegin)
            {
                func(*itBegin, startPos);
            }
        }

        template <typename RankedIndex, typename Func>
        static void reverse_for_each_on(const RankedIndex &index, size_t startPos, size_t endPos, Func &&func)
        {
            validate_range(index, startPos, endPos);
            auto range = util::reverse_range(startPos, endPos, index.size());

            auto it0 = boost::make_reverse_iterator(index.nth(range.first));
            auto it1 = boost::make_reverse_iterator(index.nth(range.second));

            for (auto itBegin = it0; startPos < endPos && itBegin != it1; ++startPos, ++itBegin)
            {
                func(*itBegin, startPos);
            }
        }

        template <typename Func, typename Element>
        static void do_each(Func &&func, const Element &element, size_t index)
        {
            if constexpr (std::is_invocable_v<Func &&, const Element &, size_t>)
            {
                func(element, index);
            }
            else if constexpr (std::is_invocable_v<Func &&, const Element &>)
            {
                func(element);
            }
        }
    };
}