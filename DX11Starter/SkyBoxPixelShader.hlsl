
// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 sampleDir	: TEXCOORD;
};


// Texture-related variables
TextureCube SkyBoxTexture		: register(t0);//the .dds skybox texture
SamplerState BasicSampler	: register(s0);// get samples from the above




float4 main(VertexToPixel input) : SV_TARGET
{
	return SkyBoxTexture.Sample(BasicSampler, input.sampleDir);//get a sample from the box in a vector3 direction
}