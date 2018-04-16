
// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

// Out of the vertex shader (and eventually input to the PS)
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 sampleDir	: TEXCOORD;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	matrix viewNoTranslation = view;
	viewNoTranslation._41 = 0;
	viewNoTranslation._42 = 0;
	viewNoTranslation._43 = 0;

	// Calculate output position
	matrix viewProj = mul(viewNoTranslation, projection);
	output.position = mul(float4(input.position, 1.0f), viewProj);

	// Push the depth of the vertex to the far clip plane
	output.position.z = output.position.w;

	// Determine my sample direction for when we 
	// sample from the cube map in the pixel shader
	output.sampleDir = input.position;
	// Assuming the position is also a direction in space towards that vertex


	return output;
}