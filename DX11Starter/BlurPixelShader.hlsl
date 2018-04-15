
cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	int blurAmount, Bleft, Bright, Bup, Bdown;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Pixels		: register(t0);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Keep track of total color
	float4 totalColor = float4(0,0,0,0);
	uint numSamples = 0;

	// Loop the requested number of times, both left and right
	// or up and down
	for (int y = Bdown; y <= Bup; y++)
	{
		for (int x = Bleft; x <= Bright; x++)
		{
			float2 uv = input.uv + float2(x * pixelWidth, y * pixelHeight);
			totalColor += Pixels.Sample(Sampler, uv);

			numSamples++;
		}
	}

	return totalColor / numSamples;
}