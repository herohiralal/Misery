#include "FullScreenBlit.common.hlsl"

float4 main(VSOutput i)
    : SV_Target
{
    return G_SrcTexture.Sample(G_SrcTextureSampler, i.uv);
}
