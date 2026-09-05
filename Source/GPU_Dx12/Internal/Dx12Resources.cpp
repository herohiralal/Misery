#include "Dx12Private.h"

#if GPU_DX12

void GPU_Dx12NewBuffer(GPU_Buffer* outBaseBuffer, GPU_Instance* baseRenderer, GPU_BufferCfg cfg) { MSR_ASSERT(false); }
void GPU_Dx12DeleteBuffer(GPU_Buffer* baseBuffer) { MSR_ASSERT(false); }
Slice_(u8) GPU_Dx12GetMappedBufferData(GPU_Buffer* baseBuffer) { MSR_ASSERT(false); return Slice_(u8) { }; }

void GPU_Dx12NewTexture(GPU_Texture* outBaseTexture, GPU_Instance* baseRenderer, GPU_TextureCfg cfg) { MSR_ASSERT(false); }
void GPU_Dx12DeleteTexture(GPU_Texture* baseTexture) { MSR_ASSERT(false); }
Slice_(u8) GPU_Dx12GetMappedTextureData(GPU_Texture* baseTexture) { MSR_ASSERT(false); return Slice_(u8) { }; }

#endif
