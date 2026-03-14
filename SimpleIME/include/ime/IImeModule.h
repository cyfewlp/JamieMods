//
// Created by jamie on 2026/1/8.
//

#pragma once

namespace Ime
{
class IImeModule
{
public:
    enum class Result
    {
        SUCCESS,
        FAILED,
        DISABLED,
        INVALID_ARGS
    };

    virtual ~IImeModule() = default;

    [[nodiscard]] virtual auto EnableIme(bool enable) -> Result = 0;
    [[nodiscard]] virtual auto ForceFocusIme() -> Result        = 0;
    [[nodiscard]] virtual auto SyncImeState() -> Result         = 0;
    [[nodiscard]] virtual auto TryFocusIme() -> Result          = 0;

    static bool IsSuccess(const Result res)
    {
        return res == Result::SUCCESS;
    }

    static bool IsFailed(const Result res)
    {
        return res == Result::FAILED;
    }
};
} // namespace Ime
