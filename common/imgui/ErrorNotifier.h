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
        std::string time;
        std::string text;
        bool        confirmed = false;
    };

    class ErrorNotifier
    {
    public:
        std::deque<ErrorMsg> errors;
        const size_t         MaxMessages = 64;

        void addError(const std::string &txt)
        {
            if (errors.size() >= MaxMessages) errors.pop_front();
            errors.push_back({currentTime(), txt, false});
        }

        void clearConfirmed()
        {
            errors.erase(std::ranges::remove_if(errors,
                                                [](const ErrorMsg &e) {
                                                    return e.confirmed;
                                                })
                             .begin(),
                         errors.end());
        }

        void Show();

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
