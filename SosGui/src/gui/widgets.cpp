//
// Created by jamie on 2025/4/30.
//
#include "gui/widgets.h"

#include "common/imgui/ImGuiFlags.h"
#include "common/imgui/ImGuiScope.h"

#include <vector>

namespace LIBC_NAMESPACE_DECL
{
namespace widgets
{

bool DrawNearActorsCombo(const std::vector<RE::Actor *> &nearActors, RE::Actor **selectedActor, RE::Actor *defaultActor)
{
    if (*selectedActor == nullptr)
    {
        *selectedActor = defaultActor;
    }
    const RE::FormID selectedFormID = (*selectedActor)->GetFormID();

    bool clicked = false;
    if (const char *previewActor = (*selectedActor)->GetName();
        ImGui::BeginCombo("##NearActors", previewActor, ImGuiUtil::ComboFlags().WidthFitPreview().HeightRegular()))
    {
        for (const auto &nearActor : nearActors)
        {
            ImGui::PushID(nearActor->formID);
            if (ImGui::Selectable(nearActor->GetName(), selectedFormID == nearActor->GetFormID()))
            {
                *selectedActor = nearActor;
                clicked        = true;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return clicked;
}
}
}