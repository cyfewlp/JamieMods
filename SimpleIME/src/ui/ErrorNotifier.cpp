//
// Created by jamie on 2025/5/5.
//

#include "ui/ErrorNotifier.h"
#include "imgui.h"

#include <ctime>

namespace LIBC_NAMESPACE_DECL
{
    void ErrorNotifier::Show()
    {
        if (errors.empty())
        {
            return;
        }
        ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);
        auto displaySize = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos({0, displaySize.y - 250}, ImGuiCond_Always);

        if (!ImGui::Begin("ErrorNotifier", nullptr,
                          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Clear"))
        {
            errors.clear();
        }

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
