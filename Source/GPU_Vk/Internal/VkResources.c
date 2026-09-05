#include "GPU_Vk/VkFns.h"
#include "VkPrivate.h"

#if GPU_VK

VmaAllocationCreateInfo GPU_VkGetVmaAllocationCreateInfo(GPU_MemType memType)
{
    VmaAllocationCreateInfo aci = {0};
    switch (memType)
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
    return aci;
}

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
    if (cfg.usages & GPU_BufUsg_ReadOnly) usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (cfg.usages & GPU_BufUsg_ReadWrite) usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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

    VmaAllocationCreateInfo aci = GPU_VkGetVmaAllocationCreateInfo(cfg.memType);

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

Slice_(u8) GPU_VkGetMappedBufferData(GPU_Buffer* baseBuffer)
{
    GPU_VkBuffer* buffer = GPU_ToVkBuffer(baseBuffer);
    MSR_ASSERT(buffer && "buffer must not be null");
    MSR_ASSERT(buffer->memType != GPU_MemType_GPU && "buffer must not be in GPU_MemType_GPU; use a staging buffer instead");
    MSR_ASSERT(buffer->mappedPtr && "buffer memory not mapped (?)");
    return (Slice_(u8)) {.data = (u8*) buffer->mappedPtr, .count = (isize) buffer->size};
}

void GPU_VkNewTexture(GPU_Texture* outBaseTexture, GPU_Instance* baseRenderer, GPU_TextureCfg cfg)
{
    GPU_VkInstance* renderer = GPU_ToVkInstance(baseRenderer);
    if (!renderer)
        return;

    MSR_ASSERT(!!outBaseTexture && "outBaseTexture can't be null");
    outBaseTexture->base.type = GPU_GfxAPIType_Vk;
    GPU_VkTexture* output = GPU_ToVkTexture(outBaseTexture);
    MSR_ASSERT(output && "output must not be null");

    if (!cfg.width)  cfg.width  = 4;
    if (!cfg.height) cfg.height = 4;
    MSR_ASSERT(cfg.format != GPU_TexFmt_Unknown && "Texture format must be specified");
    MSR_ASSERT(cfg.usages && "Texture must have at least one usage");

    output->renderer = renderer;
    output->width    = cfg.width;
    output->height   = cfg.height;
    output->memType  = cfg.memType;
    output->usages   = cfg.usages;
    output->format   = cfg.format;

    VkFormat format  = GPU_BreakVkTextureFormat(cfg.format);
    VkImageAspectFlags aspects = cfg.usages & GPU_TexUsg_DepthStencil
        ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
        : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageUsageFlags usageFlags = 0;
    if (cfg.usages & GPU_TexUsg_ReadOnly)     usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (cfg.usages & GPU_TexUsg_ReadWrite)    usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (cfg.usages & GPU_TexUsg_DrawTarget)   usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (cfg.usages & GPU_TexUsg_DepthStencil) usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (cfg.usages & GPU_TexUsg_CopySrc)      usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (cfg.usages & GPU_TexUsg_CopyDst)      usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    MSR_ASSERT(!((cfg.usages & GPU_TexUsg_DrawTarget) && (cfg.usages & GPU_TexUsg_DepthStencil)) &&
               "A texture can't be both a color and a depth-stencil attachment");

    // host-visible images must be linear-tiled to be mappable, and linear tiling supports
    // almost nothing besides transfers/sampling on most drivers
    bool hostVisible = cfg.memType != GPU_MemType_GPU;
    MSR_ASSERT((!hostVisible ||
                !(usageFlags & (VK_IMAGE_USAGE_STORAGE_BIT |
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))) &&
               "Host-visible textures are linear-tiled; use a staging buffer instead");

    VkImageCreateInfo ici =
    {
        .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext                 = nil,
        .flags                 = 0,
        .imageType             = VK_IMAGE_TYPE_2D,
        .format                = format,
        .extent                = {.width = cfg.width, .height = cfg.height, .depth = 1},
        .mipLevels             = 1,
        .arrayLayers           = 1,
        .samples               = VK_SAMPLE_COUNT_1_BIT,
        .tiling                = hostVisible ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL,
        .usage                 = usageFlags,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nil,
        .initialLayout         = hostVisible ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                             : VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo aci = GPU_VkGetVmaAllocationCreateInfo(cfg.memType);

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocInfo = {0};

    GPU_VK_CHECKED_CALL(vmaCreateImage(renderer->vmaAllocator,
        &ici, &aci, &image, &allocation, &allocInfo));

    output->actual     = image;
    output->allocation = allocation;
    output->view       = VK_NULL_HANDLE;
    output->mappedPtr  = cfg.memType == GPU_MemType_GPU ? nil : allocInfo.pMappedData;
    output->mappedSize = cfg.memType == GPU_MemType_GPU ? 0 : allocInfo.size;

    // a view is only meaningful for shader/attachment access; pure transfer targets don't need one
    if (cfg.usages & (GPU_TexUsg_ReadOnly | GPU_TexUsg_ReadWrite | GPU_TexUsg_DrawTarget | GPU_TexUsg_DepthStencil))
    {
        VkImageViewCreateInfo ivci =
        {
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext    = nil,
            .flags    = 0,
            .image    = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format   = format,
            .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange =
            {
                .aspectMask     = aspects,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
        };

        VkImageView view = VK_NULL_HANDLE;
        GPU_VK_CHECKED_CALL(vkCreateImageView(renderer->device, &ivci, nil, &view));
        output->view = view;
    }
}

void GPU_VkDeleteTexture(GPU_Texture* baseTexture)
{
    GPU_VkTexture* texture = GPU_ToVkTexture(baseTexture);
    if (!texture->renderer)
        return;

    MSR_ASSERT(texture->actual && "texture->actual must not be null");
    MSR_ASSERT(texture->allocation && "texture->allocation must not be null; are you trying to destroy a swap-chain texture?");

    if (texture->view)
        vkDestroyImageView(texture->renderer->device, texture->view, nil);

    vmaDestroyImage(texture->renderer->vmaAllocator, texture->actual, texture->allocation);

    texture->actual     = VK_NULL_HANDLE;
    texture->allocation = VK_NULL_HANDLE;
    texture->view       = VK_NULL_HANDLE;
}

Slice_(u8) GPU_VkGetMappedTextureData(GPU_Texture* baseTexture)
{
    GPU_VkTexture* texture = GPU_ToVkTexture(baseTexture);
    MSR_ASSERT(texture && "texture must not be null");
    MSR_ASSERT(texture->memType != GPU_MemType_GPU && "texture must not be in GPU_MemType_GPU; use a staging buffer instead");
    MSR_ASSERT(texture->mappedPtr && "texture memory not mapped (?)");
    return (Slice_(u8)) {.data = (u8*) texture->mappedPtr, .count = (isize) texture->mappedSize};
}

#endif
