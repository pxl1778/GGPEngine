#include "Material.h"



Material::Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader)
{
	vertexShader = pVertexShader;
	pixelShader = pPixelShader;
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