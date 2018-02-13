#pragma once
#include "SimpleShader.h"
class Material
{
public:
	Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();

private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};

