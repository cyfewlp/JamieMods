//
// Created by jamie on 2025/5/5.
//

#ifndef ERRORNOTIFIER_H
#define ERRORNOTIFIER_H

#include "common/config.h"

#include <algorithm>
#include <deque>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
struct ErrorMsg
{
    enum class Level : std::uint8_t
    {
        debug = 0,
        warning,
        error
    };

    std::string time;
    std::string text;
    bool        confirmed = false;
    Level       level     = Level::debug;
};

class ErrorNotifier
{
    std::deque<ErrorMsg> errors;
    const size_t         MaxMessages    = 64;
    ErrorMsg::Level      m_currentLevel = ErrorMsg::Level::debug;

public:
    void addError(const std::string &txt, ErrorMsg::Level level = ErrorMsg::Level::debug);

    constexpr void Debug(const std::string &txt)
    {
        addError(txt, ErrorMsg::Level::debug);
    }

    constexpr void Warning(const std::string &txt)
    {
        addError(txt, ErrorMsg::Level::warning);
    }

    constexpr void Error(const std::string &txt)
    {
        addError(txt, ErrorMsg::Level::error);
    }

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

    void SetMessageLevel(ErrorMsg::Level level)
    {
        m_currentLevel = level;
    }

    static auto GetInstance() -> ErrorNotifier &
    {
        static ErrorNotifier instance;
        return instance;
    }

private:
    void renderMessage(const ErrorMsg &msg, int idx);

    static std::string currentTime();
};
}

#endif // ERRORNOTIFIER_H
