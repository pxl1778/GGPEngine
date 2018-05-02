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

enum CreatureState { Neutral, Happy, Angry };

class Creature
{
public:
	Creature(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	~Creature();
	void Update(float deltaTime, float totalTime);
	void Draw(ID3D11DeviceContext* context, Camera* cam, DirectionalLight* dLight, DirectionalLight* dLight2, PointLight* pLight1, ID3D11ShaderResourceView* skyBoxTexture, Projection* caustic);
	//Entities
	std::vector<GameEntity*> gameEntities;

	//creature state stuff
	CreatureState guyState;

	bool isFeeding;

private:

	//texture stuff
	ID3D11ShaderResourceView* eyeTxt_neutral;
	ID3D11ShaderResourceView* eyeTxt_angry;
	ID3D11ShaderResourceView* eyeTxt_closed;
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
	Material* eyeMat_neutral;
	Material* eyeMat_angry;
	Material* eyeMat_closed;

	//Lights
	DirectionalLight dLight1;

	
	float isFeedingDuration;

	
};

