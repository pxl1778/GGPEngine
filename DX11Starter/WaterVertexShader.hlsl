cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float4 color;
	float time;
};

struct VertexShaderInput
{
	float3 position		: POSITION;     
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;

};
struct VertexToPixel
{
	float4 position		: SV_POSITION;	
	float2 uv			: TEXCOORD0;
	float3 worldPos		: TEXCOORD1;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float4 color		: COLOR;
};

VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// The vertex's position (input.position) must be converted to world space,
	// then camera space (relative to our 3D camera), then to proper homogenous 
	// screen-space coordinates.  This is taken care of by our world, view and
	// projection matrices.  
	//
	// First we multiply them together to get a single matrix which represents
	// all of those transformations (world to view to projection space)
	matrix worldViewProj = mul(mul(world, view), projection);

	// Then we convert our 3-component position vector to a 4-component vector
	// and multiply it by our final 4x4 matrix.
	//
	// The result is essentially the position (XY) of the vertex on our 2D 
	// screen and the distance (Z) from the camera (the "depth" of the pixel)
	output.position = mul(float4(input.position, 1.0f), worldViewProj);// +float4(0, sin(input.position.z + input.position.x + (time * 5)), 0, 0);
	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;// +float3(0, sin(input.position.x), 0);
	output.normal = mul(input.normal, (float3x3)world);
	output.uv = input.uv;
	output.tangent = mul(input.tangent, (float3x3)world);

	output.color = color;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}