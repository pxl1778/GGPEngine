#pragma once
#include "SimpleShader.h"
class Material
{
public:
	Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader, ID3D11ShaderResourceView* pTexture, ID3D11ShaderResourceView* pNormal, ID3D11SamplerState* pSampler);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();
	ID3D11ShaderResourceView* GetTexture();
	ID3D11ShaderResourceView* GetNormal();
	ID3D11SamplerState* GetSampler();

private:
	SimpleVertexShader * vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11ShaderResourceView* texture;
	ID3D11ShaderResourceView* normal;
	ID3D11SamplerState* sampler;
};

