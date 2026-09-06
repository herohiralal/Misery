static const float2 positions[3] = {
    float2(0.0, 0.5),
    float2(0.5, -0.5),
    float2(-0.5, -0.5)
};

/*
static const float3 colours[3] = {
    float3(0.0, 1.0, 1.0),
    float3(1.0, 0.0, 1.0),
    float3(1.0, 1.0, 0.0)
};
*/

struct VertexOutput
{
    float4 posCS : SV_Position;
    float3 vtCol : TEXCOORD0;
};

struct MatProps
{
    float4 prevVal[3]; // .rgb used for colour; .a for lerp
    float4 newVal[3];  // .rgb used; float4 for alignment
    float4 tint;       // .rgb used; float4 for alignment
};

[[vk::binding(0, 0)]]
ConstantBuffer<MatProps> G_MatProps : register(b0, space0);
