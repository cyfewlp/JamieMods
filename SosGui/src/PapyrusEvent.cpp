#include "PapyrusEvent.h"
#include "SosCallback.h"

namespace LIBC_NAMESPACE_DECL
{
    auto PapyrusEvent::Bind(RE::BSScript::IVirtualMachine *vm) -> bool
    {
        vm->RegisterFunction("RegisterForRequireUiData", "SosGuiNative", RegisterForRequireUiData);
        vm->RegisterFunction("RegisterForRequireAddActor", "SosGuiNative", RegisterForRequireAddActor);
        vm->RegisterFunction("RegisterForRequireSetEnabled", "SosGuiNative", RegisterForRequireSetEnabled);
        vm->RegisterFunction("RegisterForRequireRemoveActor", "SosGuiNative", RegisterForRequireRemoveActor);

        SosCallback::GetInstance().BindPapyrusFunctions(vm);
        return true;
    }
}
