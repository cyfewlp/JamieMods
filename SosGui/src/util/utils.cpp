//
// Created by jamie on 2025/4/29.
//

#include "util/utils.h"

#include "common/config.h"
#include "gui/UiSettings.h"

#define STB_IMAGE_IMPLEMENTATION
#include "common/stb_image.h"

namespace LIBC_NAMESPACE_DECL
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

auto util::CreateTextureFromPng(const char *filename, int *width, int *height) -> REX::W32::ID3D11ShaderResourceView *
{
    int            channels;
    unsigned char *image_data = stbi_load(filename, width, height, &channels, 4);
    if (!image_data)
    {
        return nullptr;
    }
    // 创建 DirectX 11 纹理

    auto *device = RE::BSGraphics::Renderer::GetDevice();
    // ID3D11DeviceContext *context = reinterpret_cast<ID3D11DeviceContext *>(rendererData.context);

    REX::W32::D3D11_TEXTURE2D_DESC desc = {};
    desc.width                          = *width;
    desc.height                         = *height;
    desc.mipLevels                      = 1;
    desc.arraySize                      = 1;
    desc.format                         = REX::W32::DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.sampleDesc.count               = 1;
    desc.usage                          = REX::W32::D3D11_USAGE_DEFAULT;
    desc.bindFlags                      = REX::W32::D3D11_BIND_SHADER_RESOURCE;
    desc.cpuAccessFlags                 = 0;

    REX::W32::ID3D11Texture2D       *texture = nullptr;
    REX::W32::D3D11_SUBRESOURCE_DATA subResource;
    subResource.sysMem           = image_data;
    subResource.sysMemPitch      = desc.width * 4;
    subResource.sysMemSlicePitch = 0;
    device->CreateTexture2D(&desc, &subResource, &texture);

    REX::W32::ID3D11ShaderResourceView       *srv = nullptr;
    REX::W32::D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    srvDesc.format              = REX::W32::DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.viewDimension       = REX::W32::D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.texture2D.mipLevels = desc.mipLevels;
    device->CreateShaderResourceView(texture, &srvDesc, &srv);

    texture->Release();
    stbi_image_free(image_data);
    return srv;
}
} // namespace LIBC_NAMESPACE_DECL
