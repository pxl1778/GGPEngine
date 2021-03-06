#pragma once

#include "Vertex.h"
#include <DirectXMath.h>
#include <vector>
#include <string>
#include "SimpleShader.h"

class UIButton
{
public:
	UIButton(UIVertex* vertArray, int vertCount, int* indices, int indicesCount, ID3D11Device* device, int x, int y, int w, int h);
	~UIButton();

	void LoadShaders(ID3D11Device* device, ID3D11DeviceContext* context, std::string VS_FileName, std::string PS_FileName);
	void Draw(ID3D11DeviceContext* context, DirectX::XMFLOAT4X4 projectionMat, DirectX::XMFLOAT4X4 viewMat);

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();

	int GetIndexCount();

	// Button Rect properties
	int x;
	int y;
	int width;
	int height;
private:
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	int indicesCount;
};

