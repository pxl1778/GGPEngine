#include "Creature.h"
// For the DirectX Math library
using namespace DirectX;



Creature::Creature(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	//loading textures
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/eyetxt.png", 0, &eyeTxt_neutral);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/eyetxt_angry.png", 0, &eyeTxt_angry);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/eyetxt_closed.png", 0, &eyeTxt_closed);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/bodytxt.png", 0, &bodyTxt);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/tentacletxt.png", 0, &tentacleTxt);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/blanknormal.png", 0, &blankNormal);

	//create materials
	bodyMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), bodyTxt, blankNormal, sampler);
	bodyMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	bodyMat->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");

	eyeMat_neutral = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), eyeTxt_neutral, blankNormal, sampler);
	eyeMat_neutral->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	eyeMat_neutral->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");

	eyeMat_angry = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), eyeTxt_angry, blankNormal, sampler);
	eyeMat_angry->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	eyeMat_angry->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");

	eyeMat_closed = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), eyeTxt_closed, blankNormal, sampler);
	eyeMat_closed->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	eyeMat_closed->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");

	tentacleMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), tentacleTxt, blankNormal, sampler);
	tentacleMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	tentacleMat->GetPixelShader()->LoadShaderFile(L"ToonPixelShader.cso");

	//create meshes
	abominationBody = new Mesh("../Assets/Models/abomination1/body.obj", device);
	abominationEyeball = new Mesh("../Assets/Models/abomination1/eyeball.obj", device);
	abomincationTentacle = new Mesh("../Assets/Models/abomination1/tentacle.obj", device);

	//create geometry
	//1 body
	gameEntities.push_back(new GameEntity(abominationBody, bodyMat));
	//3 eyes
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat_neutral));
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat_neutral));
	gameEntities.push_back(new GameEntity(abominationEyeball, eyeMat_neutral));
	//gameEntities[1]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	gameEntities[2]->Translate(XMFLOAT3(-1.1f, .4f, -.8f));
	gameEntities[2]->Scale(XMFLOAT3(.7f, .7f, .7f));
	//gameEntities[2]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
	gameEntities[3]->Translate(XMFLOAT3(1.1f, .4f, -.8f));
	gameEntities[3]->Scale(XMFLOAT3(.7f, .7f, .7f));
	//gameEntities[3]->Rotate(XMFLOAT3(0, -XM_PI / 4, 0));
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

	delete eyeMat_neutral;
	delete eyeMat_closed;
	delete eyeMat_angry;
	delete tentacleMat;
	delete bodyMat;

	eyeTxt_neutral->Release();
	eyeTxt_closed->Release();
	eyeTxt_angry->Release();
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

		//animate tentacles based on mood
		switch (guyState) {
			case Neutral:
				//return to neutral state for anim 1
				//if (gameEntities[i]->GetRotation().x > 0) {
				//	gameEntities[i]->Rotate(XMFLOAT3(-2 * deltaTime, 0, 0));
				//}
				//return to neutral state for anim 2
				if (gameEntities[i]->GetRotation().x < 0) {
					gameEntities[i]->Rotate(XMFLOAT3(2 * deltaTime, 0, 0));
					gameEntities[i]->Translate(XMFLOAT3(0, -.006, 0));
					gameEntities[i]->MoveForward(.002);
				}

				break;
			case Angry:
				//anim 1- tentacles recede into body
				//if (gameEntities[i]->GetRotation().x < XM_PI / 4) {
				//	gameEntities[i]->Rotate(XMFLOAT3(2 * deltaTime, 0, 0));
				//}
				
				//anim 2- tentacles curl in defensively
				if (gameEntities[i]->GetRotation().x > -XM_PIDIV4) {
					gameEntities[i]->Rotate(XMFLOAT3(-2 * deltaTime, 0, 0));
					gameEntities[i]->Translate(XMFLOAT3(0, .006, 0));
					//printf("%.2f", gameEntities[i]->GetRotation().x); printf("\n");
					gameEntities[i]->MoveForward(-.002);
				}
				break;
			case Happy:
				//return to neutral state for anim 2
				if (gameEntities[i]->GetRotation().x < 0) {
					gameEntities[i]->Rotate(XMFLOAT3(2 * deltaTime, 0, 0));
					gameEntities[i]->Translate(XMFLOAT3(0, -.006, 0));
					gameEntities[i]->MoveForward(.002);
				}
				gameEntities[i]->Rotate(XMFLOAT3(0, 2*deltaTime, 0));
				break;
		}
	}

	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->CalculateWorldMatrix();
	}
}

//for now draw method is hard coded to accept the right amount of lights in the scene; this will need to be changed if we change the lights
void Creature::Draw(ID3D11DeviceContext * context, Camera * cam, DirectionalLight* dLight, DirectionalLight* dLight2, PointLight* pLight1)
{
	//some colors to send to shader depending on guy's mood
	XMFLOAT4 white = XMFLOAT4(1.00f, 1.0f, 1.0f, 1.0);
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.7f, 0.7f, 1.0);
	XMFLOAT4 blue = XMFLOAT4(0.8f, 0.8f, 1.0f, 1);
	XMFLOAT4 color = white;

	switch (guyState) {
		case Neutral:
			//set color
			color = white;

			//swap eye texture
			gameEntities[1]->SetMaterial(eyeMat_neutral);
			gameEntities[2]->SetMaterial(eyeMat_neutral);
			gameEntities[3]->SetMaterial(eyeMat_neutral);

			break;
		case Angry:
			//color
			color = red;

			//eye texture
			gameEntities[1]->SetMaterial(eyeMat_angry);
			gameEntities[2]->SetMaterial(eyeMat_angry);
			gameEntities[3]->SetMaterial(eyeMat_angry);
			


			break;
		case Happy:
			//color
			color = blue;

			//eye texture
			gameEntities[1]->SetMaterial(eyeMat_closed);
			gameEntities[2]->SetMaterial(eyeMat_closed);
			gameEntities[3]->SetMaterial(eyeMat_closed);



			break;
	}

	//draw all entities
	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {



		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetVertexShader()->SetData("color", &color, sizeof(XMFLOAT4));
		(*it)->Draw(context, cam);
	}
	


}
