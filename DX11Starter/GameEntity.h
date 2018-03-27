#pragma once
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "BoundingBox.h"
class GameEntity
{
public:
	GameEntity(Mesh* pMeshPointer, Material* pMaterial);
	~GameEntity();

	Mesh* GetMesh();
	Material* GetMaterial();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();
	

	void SetWorldMatrix(DirectX::XMFLOAT4X4 pWorldMatrix);
	void SetPosition(DirectX::XMFLOAT3 pPosition);
	void SetRotation(DirectX::XMFLOAT3 pRotation);
	void SetScale(DirectX::XMFLOAT3 pScale);

	void Translate(DirectX::XMFLOAT3 pTranslate);
	void Rotate(DirectX::XMFLOAT3 pRotate);
	void Scale(DirectX::XMFLOAT3 pScale);

	void TestPick(DirectX::XMVECTOR pOrigin, DirectX::XMVECTOR pDirection);

	void CalculateWorldMatrix();
	void Draw(ID3D11DeviceContext* pContext, Camera* pCam);
	void PrepareMaterial(DirectX::XMFLOAT4X4 pView, DirectX::XMFLOAT4X4 pProjection, DirectX::XMFLOAT3 pCamPosition);
private:
	Mesh* meshPointer;
	Material* material;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	BoundingBox box;
};

