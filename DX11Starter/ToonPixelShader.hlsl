
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
	float3 worldPos		: TEXCOORD1;
	float4 viewPos		: TEXCOORD2;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float4 color		: COLOR;
};

struct DirectionalLight
{
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

struct PointLight
{
	float4 DiffuseColor;
	float3 Position;
	float Attenuation;
};

cbuffer externalData : register(b0) {
	DirectionalLight dLight1;
	//DirectionalLight dLight2;
	//PointLight pLight1;
	float3 CameraPosition;
	float Time;
};

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
TextureCube SkyTexture	: register(t2);
Texture2D ProjectionTexture	: register(t3);
SamplerState Sampler : register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 dirToCamera = normalize(CameraPosition - input.worldPos);

	//normal calculation
	input.normal = normalize(input.normal);
	input.tangent = normalize(normalize(input.tangent) - dot(input.tangent, input.normal)*input.normal);
	float3 biTangent = cross(input.tangent, input.normal);
	float3x3 TBN = float3x3(input.tangent, biTangent, input.normal);
	float3 unpackedNormal = NormalTexture.Sample(Sampler, input.uv).rgb * 2.0f - 1.0f;;
	float3 finalNormal = mul(unpackedNormal, TBN);
	input.normal = normalize(finalNormal);

	//lights
	float dNdotL1 = .5 + round( saturate(dot(input.normal, -dLight1.Direction)));
	//float dNdotL2 = saturate(dot(input.normal, -dLight2.Direction));
	//clamp shadow to one of two binary values, giving hard toon shade
	//dNdotL1 = .5 + clamp(floor(dNdotL1), -1, 0);
	//if (dNdotL1 < .5) { dNdotL1 = .1; }
	//else { dNdotL1 = 1; }

	//float3 dirToPLight1 = normalize(pLight1.Position - input.worldPos);
	//float pNdotL1 = saturate(dot(input.normal, dirToPLight1));
	//float pAttenuationL1 = 1.0f / (1.0f + pLight1.Attenuation * pow(length(pLight1.Position - input.worldPos), 2.0f));
	//float3 pReflectionL1 = reflect(-dirToPLight1, input.normal);
	//float specularityL1 = pow(saturate(dot(pReflectionL1, dirToCamera)), 64) * pAttenuationL1;
	//float4 finalColor = dLight1.AmbientColor + ((dLight1.DiffuseColor * dNdotL1)) + ((dLight2.DiffuseColor * dNdotL2)) + ((pLight1.DiffuseColor * pNdotL1) * pAttenuationL1) + (specularityL1);
	float4 finalColor = dLight1.AmbientColor + ((dLight1.DiffuseColor * dNdotL1));

	//diffuse
	float4 surfaceColor = DiffuseTexture.Sample(Sampler, input.uv);

	//calculate projected texture coordinates
	float2 projectionTexCoord;	
	float2 projectionTexCoord2;
	float speed = .1f;
	projectionTexCoord.x = (input.viewPos.x / input.viewPos.w / 2.0f + .5f) + (Time*speed);
	projectionTexCoord.y = (input.viewPos.y / input.viewPos.w / 2.0f + .5f) + (Time*speed);
	projectionTexCoord2.x = (-input.viewPos.x / input.viewPos.w / 2.0f + .5f) + (Time*speed);
	projectionTexCoord2.y = (-input.viewPos.y / input.viewPos.w / 2.0f + .5f) + (0*speed);
	//check if coordinates are within the projection (in 0 to 1 range)
	//http://www.rastertek.com/dx11tut43.html tutorial i'm using 
	
	//sample the color value of the projection
	//use additive so it gets brighter like light
	float4 projectionColor = ProjectionTexture.Sample(Sampler, projectionTexCoord);
	float4 projectionColor2 = ProjectionTexture.Sample(Sampler, projectionTexCoord2);
	surfaceColor += (projectionColor/2);
	surfaceColor += (projectionColor2/2);
	


	return surfaceColor * finalColor * input.color;
}