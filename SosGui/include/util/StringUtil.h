#pragma once

#include "common/config.h"
#include "common/log.h"

#include <stringapiset.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
namespace util
{
static auto UnicodeStringCompare(const char *lhs, const char *rhs) -> bool
{
    if (lhs == nullptr || rhs == nullptr)
    {
        return false;
    }
    auto lUtf16 = SKSE::stl::utf8_to_utf16(lhs).value_or(L"");
    auto rUtf16 = SKSE::stl::utf8_to_utf16(rhs).value_or(L"");

    int result = ::CompareStringEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, lUtf16.c_str(), -1, rUtf16.c_str(), -1, NULL, NULL, NULL);
    if (result == 0)
    {
        log_error("Compare unicode string fail: {}, {}", lhs, rhs);
    }
    return result > 0 && result == CSTR_LESS_THAN;
}

struct StringCompactor
{
    bool operator()(const char *lhs, const char *rhs) const { return UnicodeStringCompare(lhs, rhs); }

    bool operator()(const std::string &lhs, const std::string &rhs) const { return UnicodeStringCompare(lhs.c_str(), rhs.c_str()); }
};
} // namespace util
} // namespace LIBC_NAMESPACE_DECL
