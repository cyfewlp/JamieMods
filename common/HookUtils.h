#pragma once

namespace HookUtils
{
    template <class Signature>
    class CallHook
    {
//        static_assert(gluino::dependent_false<Signature>, "Signature template must be a function signature.");
    };

    template <class Return, class... Args> class CallHook<Return(Args...)>
    {
    private:
        REL::Relocation<Return(Args...)> original;
        SKSE::Trampoline                 trampoline{"HookUtils CallHook"};

    public:
        inline CallHook(uintptr_t address, Return (*a_hook)(Args...))
        {
            trampoline.create(32);
            original = trampoline.write_call<5>(address, reinterpret_cast<void *>(a_hook));
        }

        inline Return operator()(Args... args) const noexcept
        {
            if constexpr (std::is_void_v<Return>)
            {
                original(args...);
            }
            else
            {
                return original(args...);
            }
        }
    };

}
