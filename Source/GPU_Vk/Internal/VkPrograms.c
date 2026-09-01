#include "VkPrivate.h"

#if GPU_VK

void GPU_VkNewProgramStage(GPU_ProgramStage* outBaseProgramStage, GPU_Instance* baseRenderer, GPU_ProgramStageCfg cfg)
{
    GPU_VkInstance* renderer = GPU_ToVkInstance(baseRenderer);
    MSR_ASSERT(renderer && "renderer must not be null");

    MSR_ASSERT(outBaseProgramStage && "outBaseProgramStage must not be null");
    outBaseProgramStage->base.type = GPU_GfxAPIType_Vk;

    GPU_VkProgramStage* output = GPU_ToVkProgramStage(outBaseProgramStage);
    MSR_ASSERT(output && "programStage must not be null");

    output->type = cfg.byteCode.stage;
    vkCreateShaderModule(renderer->device, &(VkShaderModuleCreateInfo)
    {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = cfg.byteCode.code.count,
        .pCode    = (u32*) cfg.byteCode.code.data,
    }, nil, &(output->actual));
}

void GPU_VkDeleteProgramStage(GPU_ProgramStage* baseProgramStage)
{
    GPU_VkProgramStage* programStage = GPU_ToVkProgramStage(baseProgramStage);
    MSR_ASSERT(programStage && "programStage must not be null");

    vkDestroyShaderModule(programStage->renderer->device, programStage->actual, nil);
}

#endif
