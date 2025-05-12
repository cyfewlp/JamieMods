//
// Created by jamie on 2025/5/4.
//

#include "gui/ErrorNotifier.h"

#include "imgui.h"
#include "util/ImGuiUtil.h"

namespace LIBC_NAMESPACE_DECL
{
void ErrorNotifier::show()
{
    if (errors.empty())
    {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);
    auto displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos({displaySize.x - 320, displaySize.y - 250}, ImGuiCond_Always);

    if (!ImGui::Begin(
            "ErrorNotifier",
            nullptr,
            ImGuiUtil::WindowFlags().NoDecoration().AlwaysAutoResize().NoMove().NoSavedSettings().NoFocusOnAppearing()
        ))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
    {
        errors.clear();
    }

    ImGui::BeginChild("ErrList", ImVec2(300, 200), false, ImGuiWindowFlags_HorizontalScrollbar);

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(errors.size()));
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            renderMessage(errors[i], i);
        }
    }
    clipper.End();
    clearConfirmed();

    ImGui::EndChild();
    ImGui::End();
}

void ErrorNotifier::renderMessage(const ErrorMsg &msg, int idx)
{
    ImGui::Text("[%s] %s", msg.time.c_str(), msg.text.c_str());
    ImGui::SameLine();
    if (ImGui::Button(("OK##" + std::to_string(idx)).c_str()))
    {
        errors[idx].confirmed = true;
    }
}

std::string ErrorNotifier::currentTime()
{
    const time_t now = time(nullptr);
    tm           tstruct;
    char         buf[80];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), "%X", &tstruct);
    return std::string(buf);
}
}