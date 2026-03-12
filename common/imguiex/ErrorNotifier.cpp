//
// Created by jamie on 2025/5/5.
//

#include "ErrorNotifier.h"

#include "imgui.h"
#include "imguiex_m3.h"

#include <ctime>

namespace
{
inline auto GetLevelString(ErrorMsg::Level level)
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
}
} // namespace

void ErrorNotifier::addError(std::string_view msg, const ErrorMsg::Level level)
{
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

// \todo restyle by M3
void ErrorNotifier::Show()
{
    if (errors.empty())
    {
        return;
    }

    constexpr size_t visibleMessages = 10;

    auto      &m3Styles       = ImGuiEx::M3::Context::GetM3Styles();
    const auto fontScope      = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();
    const auto width          = m3Styles.GetPixels(M3Spec::Layout::Compact::Breakpoint);
    const auto height         = m3Styles.GetLastText().currText.lineHeight * visibleMessages;
    const auto viewportHeight = ImGui::GetMainViewport()->Size.y;

    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetNextWindowPos({0.F, viewportHeight - height}, ImGuiCond_Always);

    if (!ImGui::Begin("ErrorNotifier", nullptr, ImGuiEx::WindowFlags().NoDecoration().NoSavedSettings().NoFocusOnAppearing()))
    {
        ImGui::End();
        return;
    }

    if (ImGuiEx::M3::Button("Clear all", ImGuiEx::M3::ButtonConfiguration{}.Text().XSmall()))
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
                if (static_cast<size_t>((current - message.timestamp).count()) >= (m_duration.count()))
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
    ImGuiEx::M3::TextUnformatted(std::format("[{}] {}", GetLevelString(msg.level), msg.text));
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
