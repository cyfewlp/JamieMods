//
// Created by jamie on 2025/5/5.
//

#include "ErrorNotifier.h"

#include "imgui.h"
#include "imguiex_enum_wrap.h"
#include "log.h"

#include <ctime>

namespace
{
auto GetLevelString(ErrorMsg::Level level) -> const char *
{
    switch (level)
    {
        case ErrorMsg::Level::debug:
            return "Debug";
        case ErrorMsg::Level::warning:
            return "Warning";
        case ErrorMsg::Level::error:
            return "Error";
    }
    return "";
}
} // namespace

void ErrorNotifier::addError(std::string_view msg, const ErrorMsg::Level level)
{
    switch (level)
    {
        case ErrorMsg::Level::debug:
            logger::debug("{}", msg);
            break;
        case ErrorMsg::Level::warning:
            logger::warn("{}", msg);
            break;
        case ErrorMsg::Level::error:
            logger::error("{}", msg);
            break;
    }
    if (level < m_currentLevel)
    {
        return;
    }
    if (errors.size() >= MaxMessages)
    {
        errors.pop_front();
    }
    errors.push_back({.text = std::string(msg), .level = level, .confirmed = false});
}

void ErrorNotifier::Show()
{
    if (errors.empty())
    {
        return;
    }

    constexpr size_t visibleMessages = 10;

    const auto height         = ImGui::GetFontSize() * visibleMessages;
    const auto viewportHeight = ImGui::GetMainViewport()->Size.y;

    ImGui::SetNextWindowSize(ImVec2(0.0F, height), ImGuiCond_Always);
    ImGui::SetNextWindowPos({0.F, viewportHeight - height}, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("ErrorNotifier", nullptr, ImGuiEx::WindowFlags().NoDecoration().NoSavedSettings().NoFocusOnAppearing()))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear all"))
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
            auto &message = errors[static_cast<size_t>(i)];
            ImGui::PushID(i);
            if (message.level >= m_currentLevel)
            {
                renderMessage(message, static_cast<size_t>(i));
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(current - message.timestamp);
                if (m_duration < ms)
                {
                    message.confirmed = true;
                }
            }
            ImGui::PopID();
        }
    }
    clipper.End();
    clearConfirmed();

    ImGui::End();
}

void ErrorNotifier::renderMessage(const ErrorMsg &msg, size_t idx)
{
    if (ImGui::SmallButton("x"))
    {
        errors[idx].confirmed = true;
    }
    ImGui::SameLine(0.F, 20.F);
    ImGui::TextUnformatted(std::format("[{}] {}", GetLevelString(msg.level), msg.text).c_str());
}

std::string ErrorNotifier::currentTime()
{
    const time_t now = time(nullptr);
    tm           tmStruct;
    char         buf[80];
    localtime_s(&tmStruct, &now);
    strftime(buf, sizeof(buf), "%X", &tmStruct);
    return std::string(buf);
}
