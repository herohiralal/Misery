#pragma once
#include <ExtDeps_Platform.h>

#include <StbImage.h>
#include <MeshOpt.h>
#include <SpirvReflect.h>
#include <SpirvCross.h> // load after spirv-reflect, because spirv-reflect has a better spirv.h

#include <DirectX12.h>
#include <DirectX12MemoryAllocator.h>

#include <VulkanHeaders.h>
#include <VulkanLoader.h>
#include <VulkanMemoryAllocator.h>

#include <MetalHeaders.h>

#include <DirectXShaderCompiler.h>

MSR_SUPPRESS_WARN
MSR_UNSUPPRESS_WARN
