//
// Created by jamie on 2025/4/29.
//

#include "util/utils.h"
#include "common/config.h"

namespace
LIBC_NAMESPACE_DECL
{
auto util::GetFormModFileName(const RE::TESForm *form) -> std::string_view
{
    if (const auto modFile = form->GetFile(); modFile != nullptr)
    {
        return modFile->GetFilename();
    }
    return "";
}

void util::RefreshActorArmor(RE::Actor *const selectedActor)
{
    if (selectedActor != nullptr && selectedActor->GetActorRuntimeData().currentProcess != nullptr)
    {
        if (auto *currentProcess = selectedActor->GetActorRuntimeData().currentProcess;
            currentProcess != nullptr)
        {
            currentProcess->Set3DUpdateFlag(RE::RESET_3D_FLAGS::kModel);
            selectedActor->Update3DModel();
        }
    }
}
}