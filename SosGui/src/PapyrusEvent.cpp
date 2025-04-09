#include "PapyrusEvent.h"
#include "SosCallback.h"

namespace LIBC_NAMESPACE_DECL
{
    auto PapyrusEvent::Bind(RE::BSScript::IVirtualMachine *vm) -> bool
    {
        REGISTER_FUNCTION(RegisterForRequireUiData);
        REGISTER_FUNCTION(RegisterForRequireAddActor);
        REGISTER_FUNCTION(RegisterForRequireSetEnabled);
        REGISTER_FUNCTION(RegisterForRequireRemoveActor);
        REGISTER_FUNCTION(RegisterForRequireSetAutoSwitchEnabled);
        REGISTER_FUNCTION(RegisterForRequireGetAutoSwitchEnabled);
        REGISTER_FUNCTION(RegisterForRequireGetOutfitState);
        REGISTER_FUNCTION(RegisterForRequireGetOutfitList);
        REGISTER_FUNCTION(RegisterForRequireCreateOutfit);
        REGISTER_FUNCTION(RegisterForRequireRenameOutfit);
        REGISTER_FUNCTION(RegisterForRequireGetActorArmors);
        REGISTER_FUNCTION(RegisterForRequireAddToOutfit);
        REGISTER_FUNCTION(RegisterForRequireGetOutfitArmors);
        REGISTER_FUNCTION(RegisterForRequireSetQuickslot);

        REGISTER_FUNCTION(RegisterForRequireSwapArmor);
        REGISTER_FUNCTION(RegisterForRequireRemoveArmor);

        SosCallback::GetInstance().BindPapyrusFunctions(vm);
        return true;
    }
}
