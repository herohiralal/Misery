#include "MtlPrivate.h"

#if REN_MTL

void REN_MtlCreate(REN_Instance* outBaseInstance, REN_InstanceCfg cfg)
{
    @autoreleasepool
    {
        outBaseInstance->base.type = REN_GfxAPIType_Mtl;
        REN_MtlInstance* output = REN_ToMtlInstance(outBaseInstance);
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
            NSString* appName = REN_MtlMakeNSString(cfg.appName);
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

void REN_MtlWaitTillRendererIdle(REN_Instance* baseRenderer)
{
    @autoreleasepool
    {
        const REN_MtlInstance* renderer = REN_ToMtlInstance(baseRenderer);

        if (!renderer || !(renderer->gfxQueue)) return;

        id<MTLCommandBuffer> cmdBuffer = [renderer->gfxQueue commandBuffer];
        if (!cmdBuffer)
            return;

        [cmdBuffer commit];
        [cmdBuffer waitUntilCompleted];
    }
}

void REN_MtlDestroy(REN_Instance* baseRenderer)
{
    @autoreleasepool
    {
        REN_MtlInstance* renderer = REN_ToMtlInstance(baseRenderer);

        if (!renderer) return;

        REN_MtlWaitTillRendererIdle(baseRenderer);

        [renderer->gfxQueue release];
        [renderer->device release];
    }
}

#endif
