#pragma once

namespace LIBC_NAMESPACE_DECL
{
namespace util
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

constexpr auto GetInterfaceFile(const char *fileName) -> std::string
{
    static const auto &pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    return std::format("Data/Interface/{}/{}", pluginName, fileName);
}

constexpr auto GetInterfaceFile(const std::string_view &fileName) -> std::string
{
    return GetInterfaceFile(fileName.data());
}

auto IsArmorCanDisplay(const RE::TESObjectARMO *armor) -> bool;

auto GetFormModFileName(const RE::TESForm *form) -> std::string_view;

void RefreshActorArmor(RE::Actor *const selectedActor);

auto CreateTextureFromPng(const char *filename, int *width, int *height) -> REX::W32::ID3D11ShaderResourceView *;
} // namespace util
} // namespace LIBC_NAMESPACE_DECL
