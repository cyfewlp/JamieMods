//
// Created by jamie on 2026/2/13.
//

#include "imgui_te_context.h"
#include "imgui_te_engine.h"
#include "imguiex/imguiex_m3.h"
#include "m3_test_fixture.h"

#include <gtest/gtest.h>
#include <string.h>

extern bool g_fHeadless;

namespace
{
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
    RegisterTest([](ImGuiTestEngine *engine) {
        auto *const test = IM_REGISTER_TEST(engine, kTestCategory, "listItem_should_limit_min_height");
        test->GuiFunc    = [](ImGuiTestContext *ctx) {
            auto &genericVars = ctx->GenericVars;
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            ImGuiEx::M3::ListItem([&] {
                ImGui::TextUnformatted("Small content");
            });
            genericVars.ItemSize = ImGui::GetItemRectSize();
            ImGui::End();
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        test->TestFunc = [](ImGuiTestContext *ctx) {
            const auto &genericVars       = ctx->GenericVars;
            auto       &m3Styles          = ImGuiEx::M3::Context::GetM3Styles();
            const auto  expectedMinHeight = m3Styles.GetPixels(M3Spec::List::minHeight);
            IM_CHECK_GE(genericVars.ItemSize.y, expectedMinHeight);
        };
        ImGuiTestEngine_QueueTest(engine, test);
    });

    RunLoop();
}

TEST_F(M3ListTest, listItem_should_always_cover_window_width)
{
    RegisterTest([](auto &engine) {
        auto *const test = IM_REGISTER_TEST(engine, kTestCategory, "listItem_should_always_cover_window_width");
        test->GuiFunc    = [](ImGuiTestContext *ctx) {
            auto &genericVars = ctx->GenericVars;
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            ImGuiEx::M3::ListItem("empty_list_item", [&] {});
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
    RegisterTest([](auto &engine) {
        auto *const           test             = IM_REGISTER_TEST(engine, kTestCategory, "listItem_should_auto_expand_height_with_large_content");
        static constexpr auto kLargeButtonSize = ImVec2(300, 300);
        test->GuiFunc                          = [](ImGuiTestContext *ctx) {
            auto &genericVars = ctx->GenericVars;
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            ImGuiEx::M3::ListItem("large_content_list_item", [&] {
                ImGui::Button("Large content", kLargeButtonSize);
            });
            genericVars.ItemSize = ImGui::GetItemRectSize();
            ImGui::End();
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        test->TestFunc = [](ImGuiTestContext *ctx) {
            const auto &genericVars       = ctx->GenericVars;
            auto       &m3Styles          = ImGuiEx::M3::Context::GetM3Styles();
            const auto  expectedMinHeight = m3Styles.GetPixels(M3Spec::List::paddingY) * 2 + kLargeButtonSize.y;
            IM_CHECK_GE(genericVars.ItemSize.y, expectedMinHeight);
        };
        ImGuiTestEngine_QueueTest(engine, test);
    });

    RunLoop();
}
