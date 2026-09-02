#include "VkPrivate.h"

#if GPU_VK

void GPU_VkNewProgramStage(GPU_ProgramStage* outBaseProgramStage, GPU_Instance* baseRenderer, GPU_ProgramStageByteCode bc)
{
    GPU_VkInstance* renderer = GPU_ToVkInstance(baseRenderer);
    MSR_ASSERT(renderer && "renderer must not be null");

    MSR_ASSERT(outBaseProgramStage && "outBaseProgramStage must not be null");
    outBaseProgramStage->base.type = GPU_GfxAPIType_Vk;

    GPU_VkProgramStage* output = GPU_ToVkProgramStage(outBaseProgramStage);
    MSR_ASSERT(output && "programStage must not be null");

    output->renderer = renderer;
    output->type = bc.stage;

    // copy entry point
    MSR_ASSERT(bc.entryPoint.count < sizeof(output->entryPoint) - 1 && "entryPoint is too long to fit in the program stage");
    MEM_Copy(output->entryPoint, bc.entryPoint.data, bc.entryPoint.count);
    output->entryPoint[bc.entryPoint.count] = '\0';

    GPU_VK_CHECKED_CALL(vkCreateShaderModule(renderer->device, &(VkShaderModuleCreateInfo)
    {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize =bc.code.count,
        .pCode    = (u32*) bc.code.data,
    }, nil, &(output->actual)));

    GPU_VK_SET_OBJ_DEBUG_NAME(renderer, output->actual, "%", FMT(bc.objectName));
}

void GPU_VkDeleteProgramStage(GPU_ProgramStage* baseProgramStage)
{
    GPU_VkProgramStage* programStage = GPU_ToVkProgramStage(baseProgramStage);
    MSR_ASSERT(programStage && "programStage must not be null");

    vkDestroyShaderModule(programStage->renderer->device, programStage->actual, nil);
}

void GPU_VkNewProgram(GPU_Program* outBaseProgram, GPU_Instance* baseRenderer, GPU_ProgramCfg cfg)
{
    GPU_VkInstance* renderer = GPU_ToVkInstance(baseRenderer);
    MSR_ASSERT(renderer && "renderer must not be null");

    MSR_ASSERT(outBaseProgram && "outBaseProgram must not be null");
    outBaseProgram->base.type = GPU_GfxAPIType_Vk;

    GPU_VkProgram* output = GPU_ToVkProgram(outBaseProgram);
    MSR_ASSERT(output && "program must not be null");

    output->renderer = renderer;
    MSR_ASSERT(cfg.stages.count > 0 && "program must have at least one stage");
    GPU_VkProgramStage* firstStage = GPU_ToVkProgramStage(cfg.stages.data[0].stage);
    MSR_ASSERT(firstStage && "first stage must be valid");
    MSR_ASSERT(firstStage->type == GPU_ProgramStageType_Vertex || firstStage->type == GPU_ProgramStageType_Mesh
        && "other pipelines not supported yet");

    GPU_VkProgramStage *cmptStage = nil, *taskStage = nil, *meshStage = nil, *vertStage = nil, *fragStage = nil;

    // TODO: add support for program/material parameters
    GPU_VK_CHECKED_CALL(vkCreatePipelineLayout(renderer->device, &(VkPipelineLayoutCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nil,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nil,
    }, nil, &(output->pipelineLayout)));

    switch ((enum GPU_ProgramStageTypes) firstStage->type)
    {
        case GPU_ProgramStageType_Vertex:
        {
            vertStage = firstStage;
            MSR_ASSERT(cfg.stages.count == 2 && "vertex program must have a fragment stage");
            fragStage = GPU_ToVkProgramStage(cfg.stages.data[1].stage);
            MSR_ASSERT(fragStage && "fragment stage must be valid");
            output->type = GPU_ProgramType_VertexFragment;
            break;
        }
        case GPU_ProgramStageType_Mesh:
        {
            meshStage = firstStage;
            MSR_ASSERT(cfg.stages.count == 2 && "mesh program must have a fragment stage");
            fragStage = GPU_ToVkProgramStage(cfg.stages.data[1].stage);
            MSR_ASSERT(fragStage && "fragment stage must be valid");
            output->type = GPU_ProgramType_MeshFragment;
            break;
        }
        case GPU_ProgramStageType_Task:
        {
            taskStage = firstStage;
            MSR_ASSERT(cfg.stages.count == 3 && "task program must have a mesh and fragment stage");
            meshStage = GPU_ToVkProgramStage(cfg.stages.data[1].stage);
            MSR_ASSERT(meshStage && "mesh stage must be valid");
            fragStage = GPU_ToVkProgramStage(cfg.stages.data[2].stage);
            MSR_ASSERT(fragStage && "fragment stage must be valid");
            output->type = GPU_ProgramType_TaskMeshFragment;
            break;
        }
        default:
            MSR_ASSERT(false && "not implemented yet");
    }

    List_(VkPipelineShaderStageCreateInfo) shaderStages = COL_NewList(VkPipelineShaderStageCreateInfo, 3, MEM_temp);

    if (taskStage)
        COL_AppendToList(&shaderStages, ((VkPipelineShaderStageCreateInfo)
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_TASK_BIT_EXT,
            .module = taskStage->actual,
            .pName  = (cstring) taskStage->entryPoint,
        }));

    if (meshStage)
        COL_AppendToList(&shaderStages, ((VkPipelineShaderStageCreateInfo)
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_MESH_BIT_EXT,
            .module = meshStage->actual,
            .pName  = (cstring) meshStage->entryPoint,
        }));

    if (vertStage)
        COL_AppendToList(&shaderStages, ((VkPipelineShaderStageCreateInfo)
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertStage->actual,
            .pName  = (cstring) vertStage->entryPoint,
        }));

    if (fragStage)
        COL_AppendToList(&shaderStages, ((VkPipelineShaderStageCreateInfo)
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragStage->actual,
            .pName  = (cstring) fragStage->entryPoint,
        }));

    List_(VkDynamicState) dynamicStates = COL_NewList(VkDynamicState, 20, MEM_temp);

    COL_AppendAllToList(&dynamicStates,
        SLICE(VkDynamicState,
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            // VK_DYNAMIC_STATE_DEPTH_BIAS,
            // VK_DYNAMIC_STATE_BLEND_CONSTANTS,
            // VK_DYNAMIC_STATE_STENCIL_REFERENCE,
            VK_DYNAMIC_STATE_CULL_MODE, // front face will always be clockwise, and we'll use this dynamic state to flip the cull mode
        )
    );

    if (output->type == GPU_ProgramType_VertexFragment)
    {
        COL_AppendAllToList(&dynamicStates,
            SLICE(VkDynamicState,
                VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
            )
        );
    }

    Slice_(VkFormat) colourFormats = COL_NewSlice(VkFormat, cfg.targetFormats.draw.count, true, MEM_temp);
    for (usize i = 0; i < cfg.targetFormats.draw.count; i++)
        colourFormats.data[i] = GPU_BreakVkTextureFormat(cfg.targetFormats.draw.data[i]);

    GPU_VK_CHECKED_CALL(vkCreateGraphicsPipelines(renderer->device, nil, 1, &(VkGraphicsPipelineCreateInfo)
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = (u32) shaderStages.count,
        .pStages = shaderStages.data,

        // spec says these will be ignored if we have mesh shading stages
        // so no need to explicitly mark them as nil for that case
        .pVertexInputState = &(VkPipelineVertexInputStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nil,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nil,
        },
        .pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        },

        .pViewportState = &(VkPipelineViewportStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        },
        .pRasterizationState = &(VkPipelineRasterizationStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT, // front face will always be clockwise, and we'll use the dynamic state to flip the cull mode
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasSlopeFactor = 1.0f,
            .lineWidth = 1.0f,
        },
        .pMultisampleState = &(VkPipelineMultisampleStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        },
        .pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE,
        },
        .pColorBlendState = &(VkPipelineColorBlendStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &(VkPipelineColorBlendAttachmentState)
            {
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            },
        },
        .pDynamicState = &(VkPipelineDynamicStateCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = (u32) dynamicStates.count,
            .pDynamicStates = dynamicStates.data,
        },
        .layout = output->pipelineLayout,
        .renderPass = nil,
        .pNext = &(VkPipelineRenderingCreateInfo)
        {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount    = colourFormats.count,
            .pColorAttachmentFormats = colourFormats.data,
            .depthAttachmentFormat   = GPU_BreakVkTextureFormat(cfg.targetFormats.depthStencil),
            .stencilAttachmentFormat = GPU_BreakVkTextureFormat(cfg.targetFormats.depthStencil),
        },
    }, nil, &(output->actual)));

    GPU_VK_SET_OBJ_DEBUG_NAME(renderer, output->pipelineLayout, "pplnlayout_%", FMT(cfg.objectName));
    GPU_VK_SET_OBJ_DEBUG_NAME(renderer, output->actual, "ppln_%", FMT(cfg.objectName));
}

void GPU_VkDeleteProgram(GPU_Program* baseProgram)
{
    GPU_VkProgram* program = GPU_ToVkProgram(baseProgram);
    MSR_ASSERT(program && "program must not be null");

    vkDestroyPipeline(program->renderer->device, program->actual, nil);
    vkDestroyPipelineLayout(program->renderer->device, program->pipelineLayout, nil);
}

#endif
