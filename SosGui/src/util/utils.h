#pragma once

namespace SosGui::util
{
constexpr auto reverse_range(size_t start, size_t end, size_t itemCount) -> std::pair<size_t, size_t>
{
    start             = itemCount - start;
    const auto length = end - start;
    end               = start < length ? 0 : start - length;
    return {start, end};
}

constexpr auto IsArmorHasAnySlotOf(const RE::TESObjectARMO *armor, RE::BIPED_MODEL::BipedObjectSlot slot)
{
    return armor->bipedModelData.bipedObjectSlots.any(slot);
}

constexpr auto IsArmorHasNoneSlotOf(const RE::TESObjectARMO *armor, RE::BIPED_MODEL::BipedObjectSlot slot)
{
    return armor->bipedModelData.bipedObjectSlots.none(slot);
}

auto IsArmorCanDisplay(const RE::TESObjectARMO *armor) -> bool;

auto GetFormModFileName(const RE::TESForm *form) -> std::string_view;

void RefreshActorArmor(RE::Actor *const selectedActor);

} // namespace SosGui::util
