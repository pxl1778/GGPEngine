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

	void Update(float deltaTime);
	void UpdateProjectionMatrix(unsigned pWidth, unsigned pHeight);
	void UpdateRotation(float pX, float pY);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetDirection();
	float GetRotationX();
	float GetRotationY();
private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 direction;
	float rotationX;
	float rotationY;
};

