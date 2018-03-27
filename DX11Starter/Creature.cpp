#include "Creature.h"
// For the DirectX Math library
using namespace DirectX;


Creature::Creature(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	//loading textures
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/eyetxt.png", 0, &eyeTxt);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/bodytxt.png", 0, &bodyTxt);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/tentacletxt.png", 0, &tentacleTxt);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/blanknormal.png", 0, &blankNormal);

	//create materials
	bodyMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), bodyTxt, blankNormal, sampler);
	bodyMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	bodyMat->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
	eyeMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), eyeTxt, blankNormal, sampler);
	eyeMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	eyeMat->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
	tentacleMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), tentacleTxt, blankNormal, sampler);
	tentacleMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	tentacleMat->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");

	//create meshes
	abominationBody = new Mesh("../Assets/Models/abomination1/body.obj", device);
	abominationEyeball = new Mesh("../Assets/Models/abomination1/eyeball.obj", device);
	abomincationTentacle = new Mesh("../Assets/Models/abomination1/tentacle.obj", device);

	//create geometry
	//1 body
	gameEntities.push_back(new GameEntity(abominationBody, bodyMat));
	//3 eyes
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat));
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat));
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat));
	gameEntities[1]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	gameEntities[2]->Translate(XMFLOAT3(-1.1f, .4f, -.8f));
	gameEntities[2]->Scale(XMFLOAT3(.7f, .7f, .7f));
	gameEntities[2]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	gameEntities[3]->Translate(XMFLOAT3(1.1f, .4f, -.8f));
	gameEntities[3]->Scale(XMFLOAT3(.7f, .7f, .7f));
	gameEntities[3]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	//8 tentacles
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities[5]->Rotate(XMFLOAT3(0, XM_PI, 0));				
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities[6]->Rotate(XMFLOAT3(0, XM_PI / 4, 0));			
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities[7]->Rotate(XMFLOAT3(0, 3 * XM_PI / 4, 0));		
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities[8]->Rotate(XMFLOAT3(0, XM_PI / 2, 0));			
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities[9]->Rotate(XMFLOAT3(0, 5 * XM_PI / 4, 0));		
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities[10]->Rotate(XMFLOAT3(0, 3 * XM_PI / 2, 0));	
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat));
	gameEntities[11]->Rotate(XMFLOAT3(0, 7 * XM_PI / 4, 0));

	//models are big, scale em down
	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->Scale(XMFLOAT3(.5, .5, .5));
	}

}


Creature::~Creature()
{
	delete abominationBody;
	delete abominationEyeball;
	delete abomincationTentacle;

	delete eyeMat;
	delete tentacleMat;
	delete bodyMat;

	eyeTxt->Release();
	bodyTxt->Release();
	tentacleTxt->Release();
	blankNormal->Release();

	while (!gameEntities.empty()) {
		delete gameEntities.back();
		gameEntities.pop_back();
	}
	gameEntities.clear();
}

void Creature::Update(float deltaTime, float totalTime)
{
	//a little bit of hover; have separate categories of parts hover at different times to give weight/flow
	float multiplier = .001f;
	float offset = .1f;

	//body hover
	gameEntities[0]->Translate(XMFLOAT3(0, sin(totalTime) * multiplier, 0));
	//eyball hover
	gameEntities[1]->Translate(XMFLOAT3(0, sin(totalTime + (2 * offset)) * multiplier, 0)); //first eyeball is ahead of the other two
	for (int i = 2; i <= 3; i++) {
		gameEntities[i]->Translate(XMFLOAT3(0, sin(totalTime + offset)*multiplier, 0));
	}
	//tentacle hover
	for (int i = 4; i <= 11; i++) {
		gameEntities[i]->Translate(XMFLOAT3(0, sin(totalTime - offset)*multiplier, 0));
	}

	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->CalculateWorldMatrix();
	}
}

//for now draw method is hard coded to accept the right amount of lights in the scene; this will need to be changed if we change the lights
void Creature::Draw(ID3D11DeviceContext * context, Camera * cam, DirectionalLight* dLight1, DirectionalLight* dLight2, PointLight* pLight1)
{
	
	//draw all entities
	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight2", &dLight2, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetPixelShader()->SetData("pLight1", &pLight1, sizeof(PointLight));
		(*it)->Draw(context, cam);
	}
	


}
