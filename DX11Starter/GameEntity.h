#pragma once
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "DirectXCollision.h"
class GameEntity
{
public:
	GameEntity(Mesh* pMeshPointer, Material* pMaterial, std::string pName);
	~GameEntity();

	Mesh* GetMesh();
	Material* GetMaterial();
	void SetMaterial(Material* mat);
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();
	DirectX::BoundingBox* GetBoundingBox();	

	void SetWorldMatrix(DirectX::XMFLOAT4X4 pWorldMatrix);
	void SetPosition(DirectX::XMFLOAT3 pPosition);
	void SetRotation(DirectX::XMFLOAT3 pRotation);
	void SetScale(DirectX::XMFLOAT3 pScale);

	void Translate(DirectX::XMFLOAT3 pTranslate);
	void Rotate(DirectX::XMFLOAT3 pRotate);
	void Scale(DirectX::XMFLOAT3 pScale);
	void MoveForward(float pForward);

	float TestPick(DirectX::XMFLOAT3 pOrigin, DirectX::XMFLOAT3 pDirection);
	std::string GetName();

	void CalculateWorldMatrix();
	void Draw(ID3D11DeviceContext* pContext, Camera* pCam);
	void PrepareMaterial(DirectX::XMFLOAT4X4 pView, DirectX::XMFLOAT4X4 pProjection, DirectX::XMFLOAT3 pCamPosition);
private:
	Mesh* meshPointer;
	Mesh* debugBox;
	Material* material;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 forward;
	DirectX::BoundingBox* box;
	std::string name;
	void RecalculateBox();

};

