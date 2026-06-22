#pragma once
#include "../__Brahma/Brahma.h"

BRAHMA_IMPLEMENT_LIBRARY(Ext_DirectXShaderCompiler)
{
    brahma_append_string_to_paged_list(&library->interfaceDependencies, "__init");

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
    else if (package->platform == BRAHMA_PLATFORM_OSX)
    {
        Brahma_String_KVP dxcExec = {brahma_sprintf("Dependencies/DXC/osx-%s/dxc-3.7",             arch), "DXC/dxc"};
        Brahma_String_KVP dxComp  = {brahma_sprintf("Dependencies/DXC/osx-%s/libdxcompiler.dylib", arch), "DXC/libdxcompiler.dylib"};
        Brahma_String_KVP dxil    = {brahma_sprintf("Dependencies/DXC/osx-%s/libdxil.dylib",       arch), "DXC/libdxil.dylib"};
        Brahma_String_KVP llvmLic = {brahma_sprintf("Dependencies/DXC/LICENSE-LLVM.txt",           arch), "DXC/LICENSE-LLVM.txt"};
        Brahma_String_KVP msLic   = {brahma_sprintf("Dependencies/DXC/LICENSE-MS.txt",             arch), "DXC/LICENSE-MS.txt"};

        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxcExec);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxComp);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, dxil);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, llvmLic);
        brahma_append_files_to_copy_to_paged_list(&library->filesToCopyNextToOutput, msLic);
    }
}
