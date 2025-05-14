#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Setting
{
void *UiSettingReadOpenFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name);
void  UiSettingReadLineFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line);
void  UiSettingWriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf);
void  UiSettingClearAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler);

/**
 * called in ImGui::NewFrame
 */
void UiSettingApplyAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler) noexcept(false);
}
}