#pragma once

#include <windows.h>
#define NUMHOOKS      1
#define GET_MSG_PROC  0

#define MAKE_HOOK(name, id, offset, Func)                                                                              \
    struct name                                                                                                        \
    {                                                                                                                  \
        static const inline auto Address = GetAddress(id, offset);                                                     \
        using FuncType                   = Func;                                                                       \
    }

namespace SimpleIME
{
    static inline HINSTANCE hinst;
    static inline DWORD mainThreadId;

    static std::uintptr_t   GetAddress(REL::RelocationID id, REL::VariantOffset offset)
    {
        return id.address() + offset.offset();
    }

    namespace Hooks
    {
        MAKE_HOOK(D3DInit, REL::RelocationID(75595, 77226), REL::VariantOffset(0x9, 0x275, 0x00), void());
        MAKE_HOOK(D3DPresent, REL::RelocationID(75461, 77246), REL::VariantOffset(0x9, 0x9, 0x00), void(std::uint32_t));
        MAKE_HOOK(DispatchInputEvent, REL::RelocationID(67315, 68617), /**/
                  REL::VariantOffset(0x7B, 0x7B, 0x00),                /**/
                  void(RE::BSTEventSource<RE::InputEvent *> *, RE::InputEvent **));

        // Windows Hook
        typedef struct _MYHOOKDATA
        {
            int      nType;
            HOOKPROC hkprc;
            HHOOK    hhook;
        } MYHOOKDATA;

        LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam);
        void             InstallCreateWindowHook();
        void             InstallWindowsHooks();

        template <typename T>
        class CallHook
        {
        };

        template <typename R, typename... Args>
        class CallHook<R(Args...)>
        {
        public:
            inline CallHook(std::uintptr_t a_address, R (*funcPtr)(Args...))
            {
                trampoline.create(14);
                address      = a_address;
                auto ptr     = trampoline.write_call<5>(a_address, reinterpret_cast<void *>(funcPtr));
                originalFunc = ptr;
            }

            inline ~CallHook()
            {
                uintptr_t base = REL::Module::get().base();
                logv(debug, "Detaching call hook from address 0x{:#x} (offset from image base of 0x{:#x} by 0x{:#x}...",
                     address, base, address - base);
            }

            inline R operator()(Args... args) const noexcept
            {
                if constexpr (std::is_void_v<R>)
                {
                    originalFunc(args...);
                }
                else
                {
                    return originalFunc(args...);
                }
            }

        private:
            SKSE::Trampoline            trampoline{"CallHook"};
            REL::Relocation<R(Args...)> originalFunc;
            std::uintptr_t              address;
        };
    };

} // namespace SimpleIME
