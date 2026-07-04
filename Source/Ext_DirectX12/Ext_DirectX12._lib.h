#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Ext_DirectX12)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "ExtDeps_Platform");

    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:dxguid.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:d3d12.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:dxgi.lib");
        brahma_append_string_to_paged_list(&library->externalDependencies, "global:d3dcompiler.lib");
    }

    char* arch =
        package->architecture == BRAHMA_ARCHITECTURE_X64 ? "x64" :
        package->architecture == BRAHMA_ARCHITECTURE_ARM64 ? "arm64" :
        package->architecture == BRAHMA_ARCHITECTURE_X86 ? "x86" :
        NULL;

    if (!arch)
    {
        library->error = brahma_sprintf("Unsupported architecture: %s!", BRAHMA_ARCHITECTURE_NAMES[package->architecture]);
        return;
    }

    // d3d12 agility sdk files
    if (package->outputType != BRAHMA_PACKAGE_OUTPUT_TYPE_STATIC_LIBRARY && package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        Brahma_String_KVP d3d12core      = {brahma_sprintf("Dependencies/AgilitySDK/windows-%s/D3D12Core.dll",      arch), "D3D12/D3D12Core.dll"};
        Brahma_String_KVP d3d12sdkLayers = {brahma_sprintf("Dependencies/AgilitySDK/windows-%s/d3d12SDKLayers.dll", arch), "D3D12/d3d12SDKLayers.dll"};
        Brahma_String_KVP d3dconfig      = {brahma_sprintf("Dependencies/AgilitySDK/windows-%s/d3dconfig.exe",      arch), "D3D12/d3dconfig.exe"};
        Brahma_String_KVP d3dCodeLicense = {brahma_sprintf("Dependencies/AgilitySDK/LICENSE-CODE.txt",              arch), "D3D12/LICENSE-CODE.txt"};
        Brahma_String_KVP license        = {brahma_sprintf("Dependencies/AgilitySDK/LICENSE.txt",                   arch), "D3D12/LICENSE.txt"};

        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3d12core);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3d12sdkLayers);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3dconfig);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3dCodeLicense);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, license);
    }
}
