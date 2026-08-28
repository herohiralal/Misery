#include "MtlPrivate.h"

#if GPU_MTL

void GPU_MtlCreate(GPU_Instance* outBaseInstance, GPU_InstanceCfg cfg)
{
    @autoreleasepool
    {
        outBaseInstance->base.type = GPU_GfxAPIType_Mtl;
        GPU_MtlInstance* output = GPU_ToMtlInstance(outBaseInstance);
        MSR_ASSERT(output && "output must not be null");

        output->appHandle = cfg.appHandle;
        {
            utf8str nameStrToUse = cfg.appName;
            if (sizeof(output->buffers.appName) < (usize) cfg.appName.count)
                nameStrToUse = STR_SubString(cfg.appName, 0, sizeof(output->buffers.appName));

            MEM_Copy(output->buffers.appName, nameStrToUse.data, nameStrToUse.count);
            output->appName = (utf8str) {.data = output->buffers.appName, .count = nameStrToUse.count};
        }

        output->device = [MTLCreateSystemDefaultDevice() retain];
        MSR_ASSERT(output->device && "Failed to create default Metal device");

        output->gfxQueue = [output->device newCommandQueue];
        MSR_ASSERT(output->gfxQueue && "Failed to create Metal command queue");

        if (cfg.appName.data && cfg.appName.count > 0)
        {
            NSString* appName = GPU_MtlMakeNSString(cfg.appName);
            [output->gfxQueue setLabel:appName];
        }

        NSString* deviceNameNs = [output->device name];
        utf8str deviceName = STR_AliasCStr(deviceNameNs ? [deviceNameNs UTF8String] : "Unknown");
        LOG_Inf(
            METAL,
            "Device: %. unified memory: %. low power: %. removable: %.",
            FMT(deviceName),
            FMT_B8((b8) [output->device hasUnifiedMemory]),
            FMT_B8((b8) [output->device isLowPower]),
            FMT_B8((b8) [output->device isRemovable])
        );
    }
}

void GPU_MtlWaitTillIdle(GPU_Instance* baseRenderer)
{
    @autoreleasepool
    {
        const GPU_MtlInstance* renderer = GPU_ToMtlInstance(baseRenderer);

        if (!renderer || !(renderer->gfxQueue)) return;

        id<MTLCommandBuffer> cmdBuffer = [renderer->gfxQueue commandBuffer];
        if (!cmdBuffer)
            return;

        [cmdBuffer commit];
        [cmdBuffer waitUntilCompleted];
    }
}

void GPU_MtlDestroy(GPU_Instance* baseRenderer)
{
    @autoreleasepool
    {
        GPU_MtlInstance* renderer = GPU_ToMtlInstance(baseRenderer);

        if (!renderer) return;

        GPU_MtlWaitTillIdle(baseRenderer);

        [renderer->gfxQueue release];
        [renderer->device release];
    }
}

#endif
