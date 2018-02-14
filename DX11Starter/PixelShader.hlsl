
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
	float3 worldPos		: TEXCOORD0;
	float3 normal		: NORMAL;
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
	DirectionalLight dLight2;
	PointLight pLight1;
	float3 CameraPosition;
};

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
	input.normal = normalize(input.normal);
	float3 dirToCamera = normalize(CameraPosition - input.worldPos);

	float dNdotL1 = saturate(dot(input.normal, -dLight1.Direction));
	float dNdotL2 = saturate(dot(input.normal, -dLight2.Direction));

	float3 dirToPLight1 = normalize(pLight1.Position - input.worldPos);
	float pNdotL1 = saturate(dot(input.normal, dirToPLight1));
	float pAttenuationL1 = 1.0f / (1.0f + pLight1.Attenuation * pow(length(pLight1.Position - input.worldPos), 2.0f));
	float3 pReflectionL1 = reflect(-dirToPLight1, input.normal);
	float specularityL1 = pow(saturate(dot(pReflectionL1, dirToCamera)), 64) * pAttenuationL1;

	float4 finalColor = /*Surface color * */ dLight1.AmbientColor + ((dLight1.DiffuseColor * dNdotL1)) + ((dLight2.DiffuseColor * dNdotL2)) + ((pLight1.DiffuseColor * pNdotL1) * pAttenuationL1) + (specularityL1);

	return finalColor;
}