#define IMGUI_DEFINE_MATH_OPERATORS

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
constexpr auto kTestCategory   = "button test";
} // namespace

struct M3ButtonTest : ImGuiEx::M3::detail::M3TestFixture
{
protected:
    void SetUp() override { Initialize(g_fHeadless); }

    void TearDown() override { Shutdown(); }
};

template <ImGuiEx::M3::Spec::TextRole Role>
auto getTextSize()
{
    return ImGuiEx::M3::Spec::TypeScaleValue::of<Role>().textSize;
};

TEST_F(M3ButtonTest, should_support_different_sizes)
{
    RegisterTest([](ImGuiTestEngine *engine) {
        auto *const test = IM_REGISTER_TEST(engine, kTestCategory, "should_support_different_sizes");

        struct ButtonSizes
        {
            ImVec2 xsmallSize;
            ImVec2 smallSize;
            ImVec2 mediumSize;
            ImVec2 largeSize;
            ImVec2 xlargeSize;
        };

        test->SetVarsDataType<ButtonSizes>();
        test->GuiFunc = [](ImGuiTestContext *ctx) {
            auto &vars = ctx->GetVars<ButtonSizes>();
            ImGui::Begin(kTestWindowName, nullptr, ImGuiWindowFlags_NoSavedSettings);
            ImGuiEx::M3::Button("##XSmall", ImGuiEx::M3::ButtonConfiguration().XSmall());
            vars.xsmallSize = ImGui::GetItemRectSize();
            ImGuiEx::M3::Button("##Small", ImGuiEx::M3::ButtonConfiguration().Small());
            vars.smallSize = ImGui::GetItemRectSize();
            ImGuiEx::M3::Button("##Medium", ImGuiEx::M3::ButtonConfiguration().Medium());
            vars.mediumSize = ImGui::GetItemRectSize();
            ImGuiEx::M3::Button("##Large", ImGuiEx::M3::ButtonConfiguration().Large());
            vars.largeSize = ImGui::GetItemRectSize();
            ImGuiEx::M3::Button("##XLarge", ImGuiEx::M3::ButtonConfiguration().XLarge());
            vars.xlargeSize = ImGui::GetItemRectSize();
            ImGui::End();
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        test->TestFunc = [](ImGuiTestContext *ctx) {
            auto &vars = ctx->GetVars<ButtonSizes>();
            ctx->SetRef(kTestWindowName);

            ctx->ItemClick("##XSmall");
            ctx->ItemClick("##Small");
            ctx->ItemClick("##Medium");
            ctx->ItemClick("##Large");
            ctx->ItemClick("##XLarge");

            IM_CHECK_EQ(vars.xsmallSize, ImVec2(24.0F, 32.0F));
            IM_CHECK_EQ(vars.smallSize, ImVec2(32.0F, 40.0F));
            IM_CHECK_EQ(vars.mediumSize, ImVec2(48.0F, 56.0F));
            IM_CHECK_EQ(vars.largeSize, ImVec2(96.0F, 96.0F));
            IM_CHECK_EQ(vars.xlargeSize, ImVec2(128.0F, 136.0F));

            ASSERT_TRUE(ctx->TestOutput->Status);
        };
        ImGuiTestEngine_QueueTest(engine, test);
    });

    RunLoop();
}
