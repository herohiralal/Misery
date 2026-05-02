static const float2 positions[3] = {
    float2(0.0, 0.5),
    float2(0.5, -0.5),
    float2(-0.5, -0.5)
};

static const float3 colours[3] = {
    float3(0.0, 1.0, 1.0),
    float3(1.0, 0.0, 1.0),
    float3(1.0, 1.0, 0.0)
};

struct VertexOutput
{
    float4 posCS : SV_Position;
    float3 vtCol : TEXCOORD0;
};
