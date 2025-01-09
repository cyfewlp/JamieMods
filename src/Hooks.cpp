#include "Hooks.h"
#include "RenderManager.h"

#include <xbyak/xbyak.h>

void
Hooks::Install()
{
    LOG(debug, "Installing Hooks...");
    InitWornFormHook::Install();
}

template<typename T>
std::uintptr_t
__install(REL::Relocation<T> hook, void* func)
{
    SKSE::AllocTrampoline(14);
    auto& trampoline = SKSE::GetTrampoline();
    return trampoline.write_call<5>(hook.address(), func);
}

std::uintptr_t
Hooks::D3DInitHook::Install(D3DInitFunc* func)
{
    REL::Relocation<std::uintptr_t> hook{ id, offset };
    return __install(hook, func);
}

std::uintptr_t
Hooks::D3DPresentHook::Install(D3DPresentFunc* func)
{
    REL::Relocation<std::uint32_t> hook{ id, offset };
    return __install(hook, func);
}

void
Hooks::InitWornFormHook::InitWornForm(RE::TESObjectARMO* a_armor,
                                      RE::TESRace* a_race,
                                      RE::BSTSmartPointer<RE::BipedAnim>* a_biped,
                                      bool isFemale)
{
    originalFunc(a_armor, a_race, a_biped, isFemale);
}

void
Hooks::InitWornFormHook::Install()
{
    REL::Relocation<std::uintptr_t> hook{ id, offset };
    originalFunc = __install(hook, InitWornForm);
}