#include "HelloTriangle.common.hlsl"

float4 main(VertexOutput inVert)
    : SV_Target
{
    float3 colour = inVert.vtCol * G_MatProps.tint.rgb;
    return float4(colour, 1.0);
}
