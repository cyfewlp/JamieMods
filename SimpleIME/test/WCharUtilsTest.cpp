#include "common/WCharUtils.h"

#include <SimpleIni.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <string>

TEST(WCharUtilsTest, ToString)
{
    std::wstring_view wstr(L"Hello World");

    const auto &str = LIBC_NAMESPACE::WCharUtils::ToString(wstr.data(), wstr.size());
    ASSERT_STREQ("Hello World", str.c_str());

    std::wstring_view utf8WStr(L"你好，世界");

    const auto &utf8Str = LIBC_NAMESPACE::WCharUtils::ToString(utf8WStr.data(), utf8WStr.size());
    ASSERT_STREQ("你好，世界", utf8Str.c_str());
}
