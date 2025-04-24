

#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix worldInvTrans;
    matrix view;
    matrix projection;
    //float4 colorTint;
};
// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	
	// Multiply the three matrices together first
    matrix wvp = mul(projection, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	// - We don't need to alter it here, but we do need to send it to the pixel shader
    output.uv = input.uv;
    output.normal = normalize(mul((float3x3) worldInvTrans, input.normal));
    output.tangent = normalize(mul((float3x3) world, input.tangent));
    output.worldPos = mul(world, float4(input.localPosition, 1.0f)).xyz;
	

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}