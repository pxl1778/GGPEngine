#pragma once
#include "DXCore.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Lights.h"
#include <DirectXMath.h>
#include "WICTextureLoader.h"
#include <vector>

class Creature
{
public:
	Creature(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	~Creature();
	void Update(float deltaTime, float totalTime);
	void Draw(ID3D11DeviceContext* context, Camera* cam, DirectionalLight* dLight, DirectionalLight* dLight2, PointLight* pLight1, ID3D11ShaderResourceView* skyBoxTexture);
	//Entities
	std::vector<GameEntity*> gameEntities;

	//creature state stuff
	enum CreatureState{Neutral, Happy, Angry};
	CreatureState guyState;

	bool isFeeding;

private:

	//texture stuff
	ID3D11ShaderResourceView * eyeTxt;
	ID3D11ShaderResourceView* bodyTxt;
	ID3D11ShaderResourceView* tentacleTxt;
	ID3D11ShaderResourceView* blankNormal;

	//Abomination meshes
	Mesh* abominationEyeball;
	Mesh* abominationBody;
	Mesh* abomincationTentacle;

	//materials
	Material* tentacleMat;
	Material* bodyMat;
	Material* eyeMat;

	//Lights
	DirectionalLight dLight1;

	
	float isFeedingDuration;

	
};

