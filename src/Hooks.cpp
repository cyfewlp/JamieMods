#include "Hooks.h"

#include <xbyak/xbyak.h>

void
Hooks::Install()
{
    LOG(debug, "Installing Hooks...");
    D3DInitHook::Install();
    D3DPresentHook::Install();
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

void
Hooks::D3DInitHook::Install()
{
    REL::Relocation<std::uintptr_t> hook{ id, offset };
    func = __install(hook, thunk);
}

void
Hooks::D3DPresentHook::Install()
{
    REL::Relocation<std::uint32_t> hook{ id, offset };
    func = __install(hook, thunk);
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