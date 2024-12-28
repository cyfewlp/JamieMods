
#include <d3d11.h>

class RenderManager {
public:
    struct WndProcHook {
        static LRESULT thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static inline WNDPROC func;
    };

    struct D3DInitHook {
        static void thunk();
        static inline REL::Relocation<decltype(&thunk)> func;

        static constexpr auto id = REL::RelocationID(75595, 77226);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x275, 0x00);  // VR unknown

        static inline std::atomic<bool> initialized = false;
    };

    struct D3DPresentHook {
        static void __fastcall thunk(std::uint32_t);
        static constexpr auto id = REL::RelocationID(75461, 77246);
        static constexpr auto offset = REL::VariantOffset(0x9, 0x9, 0x00);
        static inline REL::Relocation<decltype(thunk)> func;
        static void Install() {
            SKSE::AllocTrampoline(14);
            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<std::uint32_t> hook{id, offset};
            func = trampoline.write_call<5>(hook.address(), thunk);
        }
    };

    void install();

    void cleanup();
};