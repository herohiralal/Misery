#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Vizkaar)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Panshilar");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Muzent");
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "Dvaarpaal");

    Brahma_String_KVP def = {"VZKR_IMPLEMENTATION", ""};
    brahma_append_definition_to_paged_list(&library->internalDefinitions, def);

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
    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        Brahma_String_KVP d3d12core      = {brahma_sprintf("Dependencies/D3D12/windows-%s/D3D12Core.dll",      arch), "D3D12/D3D12Core.dll"};
        Brahma_String_KVP d3d12sdkLayers = {brahma_sprintf("Dependencies/D3D12/windows-%s/d3d12SDKLayers.dll", arch), "D3D12/d3d12SDKLayers.dll"};
        Brahma_String_KVP d3dconfig      = {brahma_sprintf("Dependencies/D3D12/windows-%s/d3dconfig.exe",      arch), "D3D12/d3dconfig.exe"};
        Brahma_String_KVP d3dCodeLicense = {brahma_sprintf("Dependencies/D3D12/LICENSE-CODE.txt",              arch), "D3D12/LICENSE-CODE.txt"};
        Brahma_String_KVP license        = {brahma_sprintf("Dependencies/D3D12/LICENSE.txt",                   arch), "D3D12/LICENSE.txt"};

        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3d12core);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3d12sdkLayers);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3dconfig);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, d3dCodeLicense);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, license);
    }

    // dxc files
    if (package->platform == BRAHMA_PLATFORM_WINDOWS)
    {
        Brahma_String_KVP dxcExec = {brahma_sprintf("Dependencies/DXC/windows-%s/dxc.exe",        arch), "DXC/dxc.exe"};
        Brahma_String_KVP dxComp  = {brahma_sprintf("Dependencies/DXC/windows-%s/dxcompiler.dll", arch), "DXC/dxcompiler.dll"};
        Brahma_String_KVP dxil    = {brahma_sprintf("Dependencies/DXC/windows-%s/dxil.dll",       arch), "DXC/dxil.dll"};
        Brahma_String_KVP llvmLic = {brahma_sprintf("Dependencies/DXC/LICENSE-LLVM.txt",          arch), "DXC/LICENSE-LLVM.txt"};
        Brahma_String_KVP msLic   = {brahma_sprintf("Dependencies/DXC/LICENSE-MS.txt",            arch), "DXC/LICENSE-MS.txt"};

        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxcExec);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxComp);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxil);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, llvmLic);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, msLic);
    }
    else if (package->platform == BRAHMA_PLATFORM_LINUX)
    {
        Brahma_String_KVP dxcExec = {brahma_sprintf("Dependencies/DXC/linux-%s/dxc",              arch), "DXC/dxc"};
        Brahma_String_KVP dxComp  = {brahma_sprintf("Dependencies/DXC/linux-%s/libdxcompiler.so", arch), "DXC/libdxcompiler.so"};
        Brahma_String_KVP dxil    = {brahma_sprintf("Dependencies/DXC/linux-%s/libdxil.so",       arch), "DXC/libdxil.so"};
        Brahma_String_KVP llvmLic = {brahma_sprintf("Dependencies/DXC/LICENSE-LLVM.txt",          arch), "DXC/LICENSE-LLVM.txt"};
        Brahma_String_KVP msLic   = {brahma_sprintf("Dependencies/DXC/LICENSE-MS.txt",            arch), "DXC/LICENSE-MS.txt"};

        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxcExec);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxComp);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxil);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, llvmLic);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, msLic);
    }
}
