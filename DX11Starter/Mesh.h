#pragma once

#include "DXCore.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <vector>
#include <iostream>
#include <fstream>

class Mesh
{
public:
	Mesh(Vertex* vertArray, int vertCount, unsigned* indices, int indicesCount, ID3D11Device* device);
	Mesh(char* pFileName, ID3D11Device* device);
	~Mesh();
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();

	int GetIndexCount();
private:
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	int indicesCount;
};

