//
// Created by jamie on 2025/5/5.
//

#include "ErrorNotifier.h"

#include "imgui.h"

#include <ctime>

namespace LIBC_NAMESPACE_DECL
{

void ErrorNotifier::addError(const std::string &txt, const ErrorMsg::Level level)
{
    if (level < m_currentLevel)
    {
        return;
    }
    if (errors.size() >= MaxMessages)
    {
        errors.pop_front();
    }
    errors.push_back({txt, false, level});
}

void ErrorNotifier::Show()
{
    if (errors.empty())
    {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);
    auto displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos({0, displaySize.y - 250}, ImGuiCond_Always);

    if (!ImGui::Begin(
            "ErrorNotifier",
            nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing
        ))
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
    auto current = std::chrono::system_clock::now();
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            auto &message = errors[i];
            if (message.level >= m_currentLevel)
            {
                renderMessage(message, i);
                if (current - message.timestamp >= m_duration)
                {
                    message.confirmed = true;
                }
            }
        }
    }
    clipper.End();
    clearConfirmed();

    ImGui::End();
}

void ErrorNotifier::renderMessage(const ErrorMsg &msg, int idx)
{
    ImGui::Text("%s", msg.text.c_str());
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
