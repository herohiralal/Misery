#include "GPU_Vk/VkFns.h"
#include "VkPrivate.h"

#if GPU_VK

void GPU_VkNewBuffer(GPU_Buffer* outBaseBuffer, GPU_Instance* baseRenderer, GPU_BufferCfg cfg)
{
    GPU_VkInstance* renderer = GPU_ToVkInstance(baseRenderer);
    if (!renderer)
        return;

    MSR_ASSERT(!!outBaseBuffer && "outBaseBuffer can't be null");
    outBaseBuffer->base.type = GPU_GfxAPIType_Vk;
    GPU_VkBuffer* output = GPU_ToVkBuffer(outBaseBuffer);
    MSR_ASSERT(output && "output must not be null");

    if (!cfg.align) cfg.align = 16;
    MSR_ASSERT(cfg.size > 0 && "Buffer size must be greater than 0");
    MSR_ASSERT(cfg.align && (cfg.align & (cfg.align - 1)) == 0 && "Buffer alignment must be a power of 2");

    output->renderer = renderer;
    output->size     = cfg.size;
    output->align    = cfg.align;
    output->memType  = cfg.memType;
    output->usages   = cfg.usages;

    VkBufferUsageFlags usageFlags = 0;
    if (cfg.usages & GPU_BufUsg_BasicRead) usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (cfg.usages & GPU_BufUsg_BasicReadWrite) usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (cfg.usages & GPU_BufUsg_IndirectDrawArgs) usageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    if (cfg.usages & GPU_BufUsg_Vertices) usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (cfg.usages & GPU_BufUsg_Indices) usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (cfg.usages & GPU_BufUsg_CopySrc) usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (cfg.usages & GPU_BufUsg_CopyDst) usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo bci =
    {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = nil,
        .size                  = cfg.size,
        .usage                 = usageFlags,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nil,
    };

    VmaAllocationCreateInfo aci;
    switch (cfg.memType)
    {
        case GPU_MemType_Default:
            aci = (VmaAllocationCreateInfo)
            {
                .usage          = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            };
            break;
        case GPU_MemType_GPU:
            aci = (VmaAllocationCreateInfo)
            {
                .usage          = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            };
            break;
        case GPU_MemType_Readback:
            aci = (VmaAllocationCreateInfo)
            {
                .usage          = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                .flags          = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                                  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                  VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            };
            break;
        default:
            MSR_ASSERT(false && "Invalid memory type");
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocInfo = {0};

    GPU_VK_CHECKED_CALL(vmaCreateBufferWithAlignment(renderer->vmaAllocator,
        &bci, &aci, cfg.align, &buffer, &allocation, &allocInfo));

    output->actual = buffer;
    output->allocation = allocation;
    output->mappedPtr = cfg.memType == GPU_MemType_GPU ? nil : allocInfo.pMappedData;
}

void GPU_VkDeleteBuffer(GPU_Buffer* baseBuffer)
{
    GPU_VkBuffer* buffer = GPU_ToVkBuffer(baseBuffer);
    if (!buffer->renderer)
        return;

    MSR_ASSERT(buffer->actual && "buffer->actual must not be null");
    MSR_ASSERT(buffer->allocation && "buffer->allocation must not be null; are you trying to destroy a swap-chain buffer?");

    vmaDestroyBuffer(buffer->renderer->vmaAllocator, buffer->actual, buffer->allocation);

    buffer->actual = VK_NULL_HANDLE;
    buffer->allocation = VK_NULL_HANDLE;
    buffer->mappedPtr = nil;
}

void GPU_VkNewTexture(GPU_Texture* outBaseTexture, GPU_Instance* baseRenderer, GPU_TextureCfg cfg)
{
    MSR_ASSERT(false && "Not implemented yet");
}

void GPU_VkDeleteTexture(GPU_Texture* baseTexture)
{
    MSR_ASSERT(false && "Not implemented yet");
}

#endif
