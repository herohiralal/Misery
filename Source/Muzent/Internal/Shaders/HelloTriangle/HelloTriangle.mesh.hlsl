#include "HelloTriangle.common.hlsl"

[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void main(out vertices VertexOutput verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);

    [unroll]
    for (uint i = 0; i < 3; ++i)
    {
        float4 posCS = float4(positions[i], 0.0, 1.0);
        posCS.x -= 0.25;

#ifndef __spirv__
        posCS.y *= -1;
#endif

        verts[i].posCS = posCS;
        verts[i].vtCol = colours[i];
    }

    // triangles
    tris[0] = uint3(0, 2, 1);
}
