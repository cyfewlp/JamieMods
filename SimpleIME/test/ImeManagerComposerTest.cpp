//
// Created by jamie on 2025/5/21.
//

#include "ime/ImeManagerComposer.h"

#include "imgui.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace LIBC_NAMESPACE::Ime;

TEST(ImeManagerComposerTest, ShouldPushAndGetFocusType)
{
    auto *manager = ImeManagerComposer::GetInstance();
    manager->PushType(FocusType::Permanent);

    ASSERT_EQ(manager.GetFocusManageType(), FocusType::Permanent);
}
