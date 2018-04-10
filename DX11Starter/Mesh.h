#pragma once

#include "DXCore.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include "DirectXCollision.h"
#include <vector>
#include <iostream>
#include <fstream>

class Mesh
{
public:
	Mesh(Vertex* vertArray, int vertCount, unsigned* indices, int indicesCount, ID3D11Device* device);
	Mesh(char* pFileName, ID3D11Device* device);
	~Mesh();
	bool TestPick(DirectX::XMVECTOR pOrigin, DirectX::XMVECTOR pDirection);
	DirectX::XMFLOAT3 getMinSize();
	DirectX::XMFLOAT3 getMaxSize();
	DirectX::XMFLOAT3 getExtents();
	DirectX::XMFLOAT3 getCenter();

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();

	int GetIndexCount();
private:
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	int indicesCount;
	DirectX::XMFLOAT3 minSize;
	DirectX::XMFLOAT3 maxSize;
	DirectX::XMFLOAT3 extents;
	DirectX::XMFLOAT3 center;
	Vertex* vertices;
	unsigned int* indicesPointer;
};

