//
// Created by jamie on 2025/5/5.
//

#ifndef ERRORNOTIFIER_H
#define ERRORNOTIFIER_H

#include <algorithm>
#include <chrono>
#include <deque>
#include <string>

struct ErrorMsg
{
    enum class Level : std::uint8_t
    {
        debug = 0,
        warning,
        error
    };

    std::string text;
    Level       level     = Level::error;
    bool        confirmed = false;

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
};

class ErrorNotifier
{
    static constexpr size_t MaxMessages = 256;

    std::deque<ErrorMsg> errors;
    ErrorMsg::Level      m_currentLevel = ErrorMsg::Level::debug;
    std::chrono::seconds m_duration     = std::chrono::seconds(LONG_LONG_MAX);

public:
    void addError(std::string_view msg, ErrorMsg::Level level = ErrorMsg::Level::debug);

    constexpr void Debug(std::string_view msg) { addError(msg, ErrorMsg::Level::debug); }

    constexpr void Warning(std::string_view msg) { addError(msg, ErrorMsg::Level::warning); }

    constexpr void Error(std::string_view msg) { addError(msg, ErrorMsg::Level::error); }

    constexpr void clearConfirmed()
    {
        errors.erase(
            std::ranges::remove_if(
                errors,
                [](const ErrorMsg &errorMsg) {
                    return errorMsg.confirmed;
                }
            ).begin(),
            errors.end()
        );
    }

    void Show();

    void SetMessageLevel(ErrorMsg::Level level) { m_currentLevel = level; }

    void SetMessageDuration(const int seconds)
    {
        size_t seconds1 = seconds < 0 ? ULONG_LONG_MAX : static_cast<size_t>(seconds);
        m_duration      = std::chrono::seconds(seconds1);
    }

    static auto GetInstance() -> ErrorNotifier &
    {
        static ErrorNotifier instance;
        return instance;
    }

private:
    void renderMessage(const ErrorMsg &msg, size_t idx);

    static std::string currentTime();
};

#endif // ERRORNOTIFIER_H
