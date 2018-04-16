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
	bodyMat->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");
	eyeMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), eyeTxt, blankNormal, sampler);
	eyeMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	eyeMat->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");
	tentacleMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), tentacleTxt, blankNormal, sampler);
	tentacleMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	tentacleMat->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");

	//create meshes
	abominationBody = new Mesh("../Assets/Models/abomination1/body.obj", device);
	abominationEyeball = new Mesh("../Assets/Models/abomination1/eyeball.obj", device);
	abomincationTentacle = new Mesh("../Assets/Models/abomination1/tentacle.obj", device);

	//create geometry
	//1 body
	gameEntities.push_back(new GameEntity(abominationBody, bodyMat, "body"));
	//3 eyes
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat, "eye1"));
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat, "eye2"));
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat, "eye3"));
	gameEntities[1]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	gameEntities[2]->Translate(XMFLOAT3(-1.1f, .4f, -.8f));
	gameEntities[2]->Scale(XMFLOAT3(.7f, .7f, .7f));
	gameEntities[2]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	gameEntities[3]->Translate(XMFLOAT3(1.1f, .4f, -.8f));
	gameEntities[3]->Scale(XMFLOAT3(.7f, .7f, .7f));
	gameEntities[3]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	//8 tentacles
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle4"));
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle5"));
	gameEntities[5]->Rotate(XMFLOAT3(0, XM_PI, 0));						   
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle6"));
	gameEntities[6]->Rotate(XMFLOAT3(0, XM_PI / 4, 0));					   
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle7"));
	gameEntities[7]->Rotate(XMFLOAT3(0, 3 * XM_PI / 4, 0));				   
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle8"));
	gameEntities[8]->Rotate(XMFLOAT3(0, XM_PI / 2, 0));					  
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle9"));
	gameEntities[9]->Rotate(XMFLOAT3(0, 5 * XM_PI / 4, 0));				 
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle10"));
	gameEntities[10]->Rotate(XMFLOAT3(0, 3 * XM_PI / 2, 0));			
	gameEntities.push_back(new GameEntity(abomincationTentacle, tentacleMat, "tentacle11"));
	gameEntities[11]->Rotate(XMFLOAT3(0, 7 * XM_PI / 4, 0));

	//models are big, scale em down
	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->Scale(XMFLOAT3(.5, .5, .5));
	}

	//let there be light
	dLight1 = DirectionalLight({ XMFLOAT4(43.0/255.0, 61.0/255.0, 91.0/255.0, 1.0f), XMFLOAT4(251.0/255.0, 252.0/255.0, 234.0/255.0, 1.0f), XMFLOAT3(1.0f, -1.0f, 0) });

	isFeeding = false;
	isFeedingDuration = 0;

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
	//temp code to switch guy states
	if (GetAsyncKeyState('0')) guyState = Angry;
	if (GetAsyncKeyState('9')) guyState = Happy;
	if (GetAsyncKeyState('8')) guyState = Neutral;


	//a little bit of hover; have separate categories of parts hover at different times to give weight/flow
	float multiplier = .001f;
	float offset = .1f;


	// check if isFeeding 'state' is active
	if (isFeeding) {
		// start feeding timer
		isFeedingDuration += deltaTime;

		// reset appropriate variables if we are done feeding
		if (isFeedingDuration > 6.28f) {
			isFeeding = false;
			isFeedingDuration = 0;
		}
		// else, while feeding, do something
		else {
			multiplier = 0.005f;
		}
	}

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
void Creature::Draw(ID3D11DeviceContext * context, Camera * cam, DirectionalLight* dLight, DirectionalLight* dLight2, PointLight* pLight1)
{
	//some colors to send to shader depending on guy's mood
	XMFLOAT4 white = XMFLOAT4(1.00, 1.0, 1.0, 1.0);
	XMFLOAT4 red = XMFLOAT4(1.0, 0.0, 0.0, 1.0);
	XMFLOAT4 blue = XMFLOAT4(0, 0, 1, 1);
	XMFLOAT4 color = white;

	switch (guyState) {
		case Neutral:
			color = white;
			break;
		case Angry:
			color = red;
			break;
		case Happy:
			color = blue;
			break;
	}

	//draw all entities
	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
		//(*it)->GetMaterial()->GetPixelShader()->SetData("dLight2", &dLight2, sizeof(DirectionalLight));
		//(*it)->GetMaterial()->GetPixelShader()->SetData("pLight1", &pLight1, sizeof(PointLight));
		(*it)->GetMaterial()->GetVertexShader()->SetData("color", &color, sizeof(XMFLOAT4));
		(*it)->Draw(context, cam);
	}
	


}
