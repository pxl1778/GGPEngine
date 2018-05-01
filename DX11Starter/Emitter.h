#pragma once
#include <DirectXMath.h>
#include "SimpleShader.h"
#include "Camera.h"

using namespace DirectX;

struct Particle {
	XMFLOAT3 Position;
	XMFLOAT4 Color;
	XMFLOAT3 StartVelocity;
	float Size;
	float Age;
};

struct ParticleVertex {
	XMFLOAT3 Position;
	XMFLOAT2 UV;
	XMFLOAT4 Color;
	float Size;
};

class Emitter
{
public:
	Emitter(
		int maxParticles,
		float particlesPerSecond,
		float lifetime,
		float startSize,
		float endSize,
		DirectX::XMFLOAT4 startColor,
		DirectX::XMFLOAT4 endColor,
		DirectX::XMFLOAT3 startVelocity,
		DirectX::XMFLOAT3 emitterPosition,
		DirectX::XMFLOAT3 emitterAcceleration,
		ID3D11Device* device,
		SimpleVertexShader* vs,
		SimplePixelShader* ps,
		ID3D11ShaderResourceView* texture
	);
	~Emitter();
	void Update(float dt);

	void UpdateSingleParticle(float dt, int index);
	void SpawnParticle();

	void CopyParticlesToGPU(ID3D11DeviceContext* context);
	void CopyOneParticle(int index);
	void Draw(ID3D11DeviceContext* context, Camera* camera);

private:
	// Emission properties
	float particlesPerSecond;
	float secondsPerParticle;
	float timeSinceEmit;

	int livingParticleCount;
	float lifetime;

	DirectX::XMFLOAT3 emitterAcceleration;
	DirectX::XMFLOAT3 emitterPosition;
	DirectX::XMFLOAT3 startVelocity;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float startSize;
	float endSize;

	// Particle array
	Particle* particles;
	int maxParticles;
	int firstDeadIndex;
	int firstAliveIndex;

	// Rendering
	ParticleVertex* localParticleVertices;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* texture;
	SimpleVertexShader* vs;
	SimplePixelShader* ps;
};

