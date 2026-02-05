//
// Created by jamie on 2025/3/2.
//

#ifndef HOOK_H
#define HOOK_H

#include <cstdint>

namespace Hooks
{
namespace Scaleform
{
class SKSE_AllowTextInputFnHandler final : public RE::GFxFunctionHandler
{
    static inline std::uint8_t g_prevTextEntryCount = 0;

public:
    void Call(Params &params) override;

    // call SKSE_AllowTextInput to allow and return its result
    static auto AllowTextInput(bool allow) -> std::uint8_t;
    // use our text-entry-count
    static void OnTextEntryCountChanged(std::uint8_t entryCount);
};

void Install();
void Uninstall();
} // namespace Scaleform

} // namespace Hooks

#endif // HOOK_H
