#include "Hooks.h"
#include "RenderManager.h"

#include <detours/detours.h>

void Hooks::Install()
{
    LOG(debug, "Installing Hooks...");
    InitWornFormHook::Install();
    Hooks::GFXLoaderHook::Install();
}

template <typename T> std::uintptr_t __install(REL::Relocation<T> hook, void *func)
{
    SKSE::AllocTrampoline(14);
    auto &trampoline = SKSE::GetTrampoline();
    return trampoline.write_call<5>(hook.address(), func);
}

std::uintptr_t Hooks::D3DInitHook::Install(D3DInitFunc *func)
{
    REL::Relocation<std::uintptr_t> hook{id, offset};
    return __install(hook, func);
}

std::uintptr_t Hooks::D3DPresentHook::Install(D3DPresentFunc *func)
{
    REL::Relocation<std::uint32_t> hook{id, offset};
    return __install(hook, func);
}

std::uintptr_t Hooks::DispatchInputEventHook::Install(RealFunc *func)
{
    REL::Relocation<std::uintptr_t> hook{id, offset};
    return __install(hook, func);
}

void Hooks::InitWornFormHook::InitWornForm(RE::TESObjectARMO *a_armor, RE::TESRace *a_race,
                                           RE::BSTSmartPointer<RE::BipedAnim> *a_biped, bool isFemale)
{
    originalFunc(a_armor, a_race, a_biped, isFemale);
}

void Hooks::InitWornFormHook::Install()
{
    REL::Relocation<std::uintptr_t> hook{id, offset};
    originalFunc = __install(hook, InitWornForm);
}

std::uintptr_t DetourAttach(REL::RelocationID funcId, void *hook)
{
    auto result                 = DetourTransactionBegin();
    result                      = DetourUpdateThread(GetCurrentThread());

    std::uintptr_t originalFunc = funcId.address();
    result                      = DetourAttach(&(PVOID &)originalFunc, hook);
    if (result == NO_ERROR)
    {
        LOG(debug, "Function hook attached successfully in transaction, committing transaction...");
    }
    else
    {
        LOG(err, "Function attach failed code: {}.", result);
    }
    result = DetourTransactionCommit();
    if (result == NO_ERROR)
    {
        LOG(debug, "Function hook to address {:#x} committed, function hook is now active.", funcId.address());
        // originalFunc = funcId.address();
    }
    else
    {
        LOG(err, "Function hook failed code: {}.", result);
    }
    return originalFunc;
}

void Hooks::GFXLoaderHook::Install()
{
    originalFunc = DetourAttach(id, MyCreateMovie);
}

RE::GFxMovieDef *Hooks::GFXLoaderHook::MyCreateMovie(RE::GFxLoader *self, char *a_filename,
                                                     RE::GFxLoader::LoadConstants a_loadConstants,
                                                     std::size_t                  a_memoryArena)
{
    auto *movieDef = originalFunc(self, a_filename, a_loadConstants, a_memoryArena);
    return movieDef;
}