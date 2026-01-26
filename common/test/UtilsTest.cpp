//
// Created by jamie on 2026/1/27.
//
#include "../utils.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <string>

TEST(UtilsTest, ShouldSkipWhenInputHexLengthNoEqual7)
{
    ASSERT_FALSE(LIBC_NAMESPACE::CommonUtils::HexToU32("#AABBCCDD").has_value());
    ASSERT_FALSE(LIBC_NAMESPACE::CommonUtils::HexToU32("").has_value());
}

TEST(UtilsTest, ShouldSkipWhenInputIsNotHexString)
{
    ASSERT_FALSE(LIBC_NAMESPACE::CommonUtils::HexToU32("#xxxxxx").has_value());
    ASSERT_FALSE(LIBC_NAMESPACE::CommonUtils::HexToU32("&112233").has_value());
}

TEST(UtilsTest, ShouldConvertToABGR)
{
    const auto opt = LIBC_NAMESPACE::CommonUtils::HexToU32("#112233");
    ASSERT_TRUE(opt.has_value());
    ASSERT_TRUE(opt.value() == 0x332211);
}
