#include "FullScreenBlit.common.hlsl"

VSOutput main(uint vertexID: SV_VertexID)
{
    VSOutput o;
    o.uv = float2((vertexID << 1) & 2, vertexID & 2);
    o.position = float4(o.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);

#ifdef __spirv__
    o.position.y *= -1;
#endif

    return o;
}
