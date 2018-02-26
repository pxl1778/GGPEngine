#include "Material.h"



Material::Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader, ID3D11ShaderResourceView* pTexture, ID3D11ShaderResourceView* pNormal, ID3D11SamplerState* pSampler)
{
	vertexShader = pVertexShader;
	pixelShader = pPixelShader;
	texture = pTexture;
	normal = pNormal;
	sampler = pSampler;

}


Material::~Material()
{
	delete vertexShader;
	delete pixelShader;
}

SimpleVertexShader* Material::GetVertexShader() {
	return vertexShader;
}

SimplePixelShader* Material::GetPixelShader() {
	return pixelShader;
}

ID3D11ShaderResourceView* Material::GetTexture() {
	return texture;
}

ID3D11ShaderResourceView* Material::GetNormal() {
	return normal;
}

ID3D11SamplerState* Material::GetSampler() {
	return sampler;
}