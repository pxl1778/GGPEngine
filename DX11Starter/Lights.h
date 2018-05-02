#pragma once
#include <DirectXMath.h>

struct DirectionalLight {
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Direction;
};

struct PointLight {
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Position;
	float Attenuation;
};

struct Projection {
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	ID3D11ShaderResourceView* projectionTexture;
};