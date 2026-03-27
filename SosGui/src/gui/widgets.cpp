//
// Created by jamie on 2025/4/30.
//
#include "gui/widgets.h"

#include "imguiex/imguiex_enum_wrap.h"

#include <vector>

namespace SosGui::widgets
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
        ImGui::BeginCombo("##NearActors", previewActor, ImGuiEx::ComboFlags().WidthFitPreview().HeightRegular()))
    {
        for (const auto &nearActor : nearActors)
        {
            ImGui::PushID(static_cast<int>(nearActor->formID));
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
} // namespace SosGui::widgets
