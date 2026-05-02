#include "HelloTriangle.common.hlsl"

VertexOutput main(uint vid: SV_VertexID)
{
    VertexOutput output;
    output.posCS = float4(positions[vid], 0.0, 1.0);
    output.posCS.x += 0.25;

#ifdef __spirv__
    output.posCS.y *= -1;
#endif

    output.vtCol = colours[vid];
    return output;
}
