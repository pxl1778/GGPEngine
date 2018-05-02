struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD0;
	float3 worldPos		: TEXCOORD1;
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

cbuffer externalData : register(b0) {
	DirectionalLight dLight1;
	float3 CameraPosition;
	float time;
};

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
TextureCube SkyTexture	: register(t2);
SamplerState Sampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 dirToCamera = normalize(CameraPosition - input.worldPos);

	//normal calculation
	input.normal = normalize(input.normal);
	input.tangent = normalize(normalize(input.tangent) - dot(input.tangent, input.normal)*input.normal);
	float3 biTangent = cross(input.tangent, input.normal);
	float3x3 TBN = float3x3(input.tangent, biTangent, input.normal);
	float2 newUV = input.uv + float2(-time, time)/35;
	float3 unpackedNormal = NormalTexture.Sample(Sampler, newUV).rgb * 2.0f - 1.0f;;
	float3 finalNormal = mul(unpackedNormal, TBN);
	float normalOffset = dot(finalNormal, input.normal);
	//input.normal = normalize(finalNormal);

	//lights
	float dNdotL1 = saturate(dot(-input.normal, -dLight1.Direction));
	float3 newLightDirection = float3(dLight1.Direction.x, -dLight1.Direction.y, dLight1.Direction.z);
	float3 newLightPosition = newLightDirection + float3(15, 0, 0);

	float3 dirToDirLight = normalize(-newLightPosition - input.worldPos);

	float specularityL1 = pow(saturate(dot(reflect(-dirToDirLight, input.normal), dirToCamera)), 32) * step(0.9f, normalOffset);
	float4 finalColor = dLight1.AmbientColor + ((dLight1.DiffuseColor * dNdotL1)) + specularityL1;

	//diffuse
	float4 surfaceColor = DiffuseTexture.Sample(Sampler, input.uv);

	return finalColor;

	//return surfaceColor * finalColor * input.color;
}