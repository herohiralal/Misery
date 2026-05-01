Texture2D G_SrcTexture;
SamplerState G_SrcTextureSampler;

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};
