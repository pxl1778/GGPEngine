#include "UIButton.h"

using namespace DirectX;

UIButton::UIButton(UIVertex* vertArray, int vertCount, int* indices, int indicesCount, ID3D11Device* device, int x, int y, int w, int h)
{
	// initialize vertex buffer and index buffer pointers to ensure there is no 'junk' data
	vertexBuffer = 0;
	indexBuffer = 0;

	this->indicesCount = indicesCount;

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(UIVertex) * vertCount;       
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertArray;

	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * indicesCount;         
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; 
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = indices;

	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);

	vertexShader = 0;
	pixelShader = 0;

	// set button's position
	this->x = x;
	this->y = y;
	this->width = w;
	this->height = h;
}


UIButton::~UIButton()
{
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }

	delete vertexShader;
	delete pixelShader;
}

void UIButton::LoadShaders(ID3D11Device * device, ID3D11DeviceContext * context, std::string VS_FileName, std::string PS_FileName)
{
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"UI_VS.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"UI_PS.cso");
}

void UIButton::Draw(ID3D11DeviceContext * context, DirectX::XMFLOAT4X4 projectionMat, DirectX::XMFLOAT4X4 viewMat)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	UINT UIstride = sizeof(UIVertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &UIstride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//vertexShader->SetMatrix4x4("view", viewMat);
	vertexShader->SetMatrix4x4("projection", projectionMat);

	vertexShader->CopyAllBufferData();

	vertexShader->SetShader();
	pixelShader->SetShader();

	context->DrawIndexed(indicesCount, // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

ID3D11Buffer * UIButton::GetVertexBuffer()
{
	return vertexBuffer;
}

ID3D11Buffer * UIButton::GetIndexBuffer()
{
	return indexBuffer;
}

SimpleVertexShader * UIButton::GetVertexShader()
{
	return vertexShader;
}

SimplePixelShader * UIButton::GetPixelShader()
{
	return pixelShader;
}

int UIButton::GetIndexCount()
{
	return indicesCount;
}
