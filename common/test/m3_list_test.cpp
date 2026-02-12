//
// Created by jamie on 2026/2/13.
//

#include "imgui_te_context.h"
#include "imgui_te_engine.h"
#include "imguiex/M3ThemeBuilder.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/m3/facade/base.h"
#include "m3_test_fixture.h"

#include <gtest/gtest.h>
#include <string.h>

namespace
{
bool           g_fHeadless     = false;
constexpr auto kTestWindowName = "ImGuiEx M3 Test";
constexpr auto kTestCategory   = "list_test";
} // namespace

struct M3ListTest : ImGuiEx::M3::detail::M3TestFixture
{
protected:
    void SetUp() override { Initialize(g_fHeadless); }

    void TearDown() override { Shutdown(); }
};

TEST_F(M3ListTest, listItem_should_limit_min_height)
{
    RegisterTest([](ImGuiTestEngine *engine, ImGuiEx::M3::M3Styles &m3Styles) {
        auto *const test = IM_REGISTER_TEST(engine, kTestCategory, "listItem_should_limit_min_height");
        test->UserData   = &m3Styles;
        test->GuiFunc    = [](ImGuiTestContext *ctx) {
            auto       &genericVars = ctx->GenericVars;
            const auto *pM3Styles   = static_cast<ImGuiEx::M3::M3Styles *>(ctx->Test->UserData);
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            ImGuiEx::M3::ListItem("small_content_list_item", *pM3Styles, [&] {
                ImGui::TextUnformatted("Small content");
            });
            genericVars.ItemSize = ImGui::GetItemRectSize();
            ImGui::End();
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        test->TestFunc = [](ImGuiTestContext *ctx) {
            const auto &genericVars       = ctx->GenericVars;
            const auto *pM3Styles         = static_cast<ImGuiEx::M3::M3Styles *>(ctx->Test->UserData);
            const auto  expectedMinHeight = pM3Styles->GetPixels(M3Spec::List::minHeight);
            IM_CHECK_GE(genericVars.ItemSize.y, expectedMinHeight);
        };
        ImGuiTestEngine_QueueTest(engine, test);
    });

    RunLoop();
}

TEST_F(M3ListTest, listItem_should_ignore_interact_when_content_is_empty)
{
    RegisterTest([](auto &engine, ImGuiEx::M3::M3Styles &m3Styles) {
        auto *const test =
            IM_REGISTER_TEST(engine, kTestCategory, "listItem_should_ignore_interact_when_content_is_empty");
        test->UserData = &m3Styles;
        test->GuiFunc  = [](ImGuiTestContext *ctx) {
            auto       &genericVars = ctx->GenericVars;
            const auto *pM3Styles   = static_cast<ImGuiEx::M3::M3Styles *>(ctx->Test->UserData);
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            if (ImGuiEx::M3::ListItem("empty_list_item", *pM3Styles, [&] {}))
            {
                genericVars.Count += 1;
            }
            ImGui::End();
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        test->TestFunc = [](ImGuiTestContext *ctx) {
            const auto &genericVars = ctx->GenericVars;
            ctx->SetRef(kTestWindowName);
            ctx->ItemClick("empty_list_item", 0, ImGuiTestOpFlags_NoCheckHoveredId);
            IM_CHECK_EQ(genericVars.Count, 0);
        };
        ImGuiTestEngine_QueueTest(engine, test);
    });

    RunLoop();
}

TEST_F(M3ListTest, listItem_should_always_cover_window_width)
{
    RegisterTest([](auto &engine, ImGuiEx::M3::M3Styles &m3Styles) {
        auto *const test = IM_REGISTER_TEST(engine, kTestCategory, "listItem_should_always_cover_window_width");
        test->UserData   = &m3Styles;
        test->GuiFunc    = [](ImGuiTestContext *ctx) {
            auto       &genericVars = ctx->GenericVars;
            const auto *pM3Styles   = static_cast<ImGuiEx::M3::M3Styles *>(ctx->Test->UserData);
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            ImGuiEx::M3::ListItem("empty_list_item", *pM3Styles, [&] {});
            genericVars.ItemSize   = ImGui::GetItemRectSize();
            genericVars.WindowSize = ImGui::GetContentRegionAvail();
            ImGui::End();
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        test->TestFunc = [](ImGuiTestContext *ctx) {
            const auto &genericVars = ctx->GenericVars;
            IM_CHECK_EQ(genericVars.ItemSize.x, genericVars.WindowSize.x);
        };
        ImGuiTestEngine_QueueTest(engine, test);
    });

    RunLoop();
}

TEST_F(M3ListTest, listItem_should_auto_expand_height_with_large_content)
{
    RegisterTest([](auto &engine, ImGuiEx::M3::M3Styles &m3Styles) {
        auto *const test =
            IM_REGISTER_TEST(engine, kTestCategory, "listItem_should_auto_expand_height_with_large_content");
        static constexpr auto kLargeButtonSize = ImVec2(300, 300);
        test->UserData                         = &m3Styles;
        test->GuiFunc                          = [](ImGuiTestContext *ctx) {
            auto       &genericVars = ctx->GenericVars;
            const auto *pM3Styles   = static_cast<ImGuiEx::M3::M3Styles *>(ctx->Test->UserData);
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            ImGuiEx::M3::ListItem("large_content_list_item", *pM3Styles, [&] {
                ImGui::Button("Large content", kLargeButtonSize);
            });
            genericVars.ItemSize = ImGui::GetItemRectSize();
            ImGui::End();
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        test->TestFunc = [](ImGuiTestContext *ctx) {
            const auto &genericVars       = ctx->GenericVars;
            const auto *pM3Styles         = static_cast<ImGuiEx::M3::M3Styles *>(ctx->Test->UserData);
            const auto  expectedMinHeight = pM3Styles->GetPixels(M3Spec::List::paddingY) * 2 + kLargeButtonSize.y;
            IM_CHECK_GE(genericVars.ItemSize.y, expectedMinHeight);
        };
        ImGuiTestEngine_QueueTest(engine, test);
    });

    RunLoop();
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    bool use_gui = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--gui") == 0) use_gui = true;
    }
    g_fHeadless = !use_gui;
    return RUN_ALL_TESTS();
}
