#pragma once
#include <DirectXMath.h>
#include "DXCore.h"
class Camera
{
public:
	Camera();
	~Camera();

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	DirectX::XMFLOAT4X4 GetWorldMatrix();

	void Update(float deltaTime);
	void UpdateLookAt(float deltaTime, DirectX::XMFLOAT3 pTargetPos);
	void UpdateProjectionMatrix(unsigned pWidth, unsigned pHeight);
	void UpdateRotation(float pX, float pY);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetDirection();
	float GetRotationX();
	float GetRotationY();
private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4X4 worldMatrix;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 direction;
	float rotationX;
	float rotationY;
};

