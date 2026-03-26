//
// Created by jamie on 2025/4/29.
//

#include "util/utils.h"

#include "gui/UiSettings.h"

namespace SosGui
{
auto util::IsArmorCanDisplay(const RE::TESObjectARMO *armor) -> bool
{
    bool canDisplay = false;
    if (armor != nullptr)
    {
        if (!Settings::UiSettings::GetInstance()->includeTemplateArmor && armor->templateArmor != nullptr)
        {
            return canDisplay;
        }
        if (const std::string_view name = armor->GetName(); !name.empty())
        {
            canDisplay = true;
        }
    }
    return canDisplay;
}

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
        if (auto *currentProcess = selectedActor->GetActorRuntimeData().currentProcess; currentProcess != nullptr)
        {
            currentProcess->Set3DUpdateFlag(RE::RESET_3D_FLAGS::kModel);
            selectedActor->Update3DModel();
        }
    }
}
} // namespace SosGui
