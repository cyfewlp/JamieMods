
#include <Windows.h>
#include <detours/detours.h>

namespace Input {
    HRESULT(WINAPI *Real_DirectInput8Create)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN) = NULL;
    static HRESULT WINAPI SE_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut,
                                                LPUNKNOWN punkOuter) {
        LOG(info, "[Detour]Before DirectInput8Create");
        auto hr = Real_DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
        LOG(info, "[Detour]After DirectInput8Create");
        return hr;
    }

    static void detour() {
        LOG(info, "try find dinput functions");
        Real_DirectInput8Create = (HRESULT(WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN))DetourFindFunction(
            "dinput8.dll", "DirectInput8Create");
        if (Real_DirectInput8Create != NULL) {
            LOG(info, "found DirectInput8Create!!!");

            LOG(info, "Detouring DirectInput8Create...");
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID &)Real_DirectInput8Create, SE_DirectInput8Create);
            HRESULT error = DetourTransactionCommit();
            if (error == NO_ERROR) {
                LOG(info, "extend" DETOURS_STRINGIFY(DETOURS_BITS) ".dll: Detoured DirectInput8Create().");
            } else {
                LOG(err, "extend" DETOURS_STRINGIFY(DETOURS_BITS) ".dll: Error detouring DirectInput8Create(): {}",
                    error);
            }
        } else
            LOG(info, "can't found DirectInput8Create.");
    }

}