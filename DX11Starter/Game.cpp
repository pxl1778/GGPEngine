#include "Game.h"
#include "Vertex.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	vertexShader = 0;
	pixelShader = 0;

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.");
#endif

	cam = new Camera();
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;

	delete m1;
	delete m2;
	delete m3;
	delete m4;
	delete m5; 
	delete m6;

	delete abominationBody;
	delete abominationEyeball;
	delete abomincationTentacle;

	delete mat1;

	while (!gameEntities.empty()) {
		delete gameEntities.back();
		gameEntities.pop_back();
	}
	gameEntities.clear();

	delete cam;
	sampler->Release();
	wallTexture->Release();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dLight1 = DirectionalLight({XMFLOAT4(.05f, .05f, .13f, 1.0f), XMFLOAT4(.4f, .3f, .9f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0)});
	dLight2 = DirectionalLight({ XMFLOAT4(0, 0, 0, 0), XMFLOAT4(.01f, .5f, .01f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	pLight1 = PointLight({ XMFLOAT4(1.0f, .1f, .1f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), .1f });
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	//Loading Textures
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/Wall Stone 004_COLOR.jpg", 0, &wallTexture);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/Wall Stone 004_NRM.jpg", 0, &wallNormal);
	D3D11_SAMPLER_DESC sd = {};
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sd, &sampler);
	//Material 1
	mat1 = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), wallTexture, wallNormal, sampler);
	mat1->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	mat1->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
	//mat1->GetPixelShader()->SetShaderResourceView("");
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//    update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//    an identity matrix.  This is just to show that HLSL expects a different
	//    matrix (column major vs row major) than the DirectX Math library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	cam->UpdateProjectionMatrix(width, height);
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	m1 = new Mesh("../Assets/Models/sphere.obj", device);
	m2 = new Mesh("../Assets/Models/helix.obj", device);
	m3 = new Mesh("../Assets/Models/cone.obj", device);
	m4 = new Mesh("../Assets/Models/cube.obj", device);
	m5 = new Mesh("../Assets/Models/cylinder.obj", device);
	m6 = new Mesh("../Assets/Models/torus.obj", device);

	abominationBody = new Mesh("../Assets/Models/abomination1/body.obj", device);
	abominationEyeball = new Mesh("../Assets/Models/abomination1/eyeball.obj", device);
	abomincationTentacle = new Mesh("../Assets/Models/abomination1/tentacle.obj", device);

	//gameEntities.push_back(new GameEntity(m1, mat1));
	//gameEntities[0]->SetPosition(XMFLOAT3(.7f, 0, 0));
	//gameEntities.push_back(new GameEntity(m2, mat1));
	//gameEntities[1]->SetPosition(XMFLOAT3(-.7f, 0, 0));
	//gameEntities.push_back(new GameEntity(m3, mat1));
	//gameEntities[2]->SetPosition(XMFLOAT3(0, 0, -1));

	//1 body
	gameEntities.push_back(new GameEntity(abominationBody, mat1));
	//3 eyes
	gameEntities.push_back(new GameEntity(abominationEyeball, mat1));
	gameEntities.push_back(new GameEntity(abominationEyeball, mat1));
	gameEntities.push_back(new GameEntity(abominationEyeball, mat1));
	gameEntities[2]->Translate(XMFLOAT3(-1.1f, .4f, -.8f));
	gameEntities[2]->Scale(XMFLOAT3(.7f, .7f, .7f));
	gameEntities[3]->Translate(XMFLOAT3(1.1f, .4f, -.8f));
	gameEntities[3]->Scale(XMFLOAT3(.7f, .7f, .7f));
	//8 tentacles
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities[5]->Rotate(XMFLOAT3(0, XM_PI, 0));
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities[6]->Rotate(XMFLOAT3(0, XM_PI/4, 0));
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities[7]->Rotate(XMFLOAT3(0, 3*XM_PI/4, 0));
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities[8]->Rotate(XMFLOAT3(0, XM_PI/2, 0));
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities[9]->Rotate(XMFLOAT3(0, 5*XM_PI/4, 0));
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities[10]->Rotate(XMFLOAT3(0, 3*XM_PI/2, 0));
	gameEntities.push_back(new GameEntity(abomincationTentacle, mat1));
	gameEntities[11]->Rotate(XMFLOAT3(0, 7*XM_PI/4, 0));

	//models are big, scale em down
	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->Scale(XMFLOAT3(.5, .5, .5));
	}
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	cam->UpdateProjectionMatrix(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	//a little bit of hover; have separate categories of parts hover at different times to give weight/flow
	float multiplier = .001f;
	float offset = .1f;

	////body hover
	//gameEntities[0]->Translate(XMFLOAT3(0, sin(totalTime) * multiplier, 0));
	////eyball hover
	//gameEntities[1]->Translate(XMFLOAT3(0, sin(totalTime + (2 * offset)) * multiplier, 0)); //first eyeball is ahead of the other two
	//for (int i = 2; i <= 3; i++) {
	//	gameEntities[i]->Translate(XMFLOAT3(0, sin(totalTime + offset)*multiplier, 0));
	//}
	////tentacle hover
	//for (int i = 4; i <= 11; i++) {
	//	gameEntities[i]->Translate(XMFLOAT3(0, sin(totalTime - offset)*multiplier, 0));
	//}

	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->CalculateWorldMatrix();
	}
	//cam->Update(deltaTime);
	cam->UpdateLookAt(deltaTime, XMFLOAT3(0, 0, 0)); //Here is where we'd pass in the creature's position
	pLight1.Position = XMFLOAT3(pLight1.Position.x, sin(totalTime) * .5f, pLight1.Position.z);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight2", &dLight2, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetPixelShader()->SetData("pLight1", &pLight1, sizeof(PointLight));
		(*it)->Draw(context, cam);
	}

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	TestInteraction(x, y);

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Check left mouse button
	if (buttonState & 0x0001)
	{
		cam->UpdateRotation((x - (float)prevMousePos.x) * 0.005f, (y - (float)prevMousePos.y) * 0.005f);
	}
	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}

void Game::TestInteraction(int pMouseX, int pMouseY) {
	//http://www.rastertek.com/dx11tut47.html
	float pointX, pointY;
	XMMATRIX inverseViewMatrix, viewMatrix;
	XMFLOAT4X4 inverseViewMatrixF, inverseEntityMatrix;
	XMFLOAT3 direction;

	pointX = ((2.0f * (float)pMouseX) / (float)width) - 1.0f;
	pointY = (((2.0f * (float)pMouseY) / (float)height) - 1.0f) * -1.0f;

	pointX = pointX / cam->GetProjectionMatrix()._11;
	pointY = pointY / cam->GetProjectionMatrix()._22;

	viewMatrix = XMLoadFloat4x4(&(cam->GetViewMatrix()));
	inverseViewMatrix = XMMatrixInverse(nullptr, viewMatrix);//could go wrong
	XMStoreFloat4x4(&inverseViewMatrixF, inverseViewMatrix);

	direction.x = (pointX * inverseViewMatrixF._11) + (pointY * inverseViewMatrixF._21) + inverseViewMatrixF._31;
	direction.y = (pointX * inverseViewMatrixF._12) + (pointY * inverseViewMatrixF._22) + inverseViewMatrixF._32;
	direction.z = (pointX * inverseViewMatrixF._13) + (pointY * inverseViewMatrixF._23) + inverseViewMatrixF._33;

	XMFLOAT3 origin = cam->GetPosition();

	XMMATRIX entityWorldMatrix = XMMatrixInverse(nullptr, XMLoadFloat4x4(&(gameEntities[0]->GetWorldMatrix())));
	XMStoreFloat4x4(&inverseEntityMatrix, entityWorldMatrix);

	XMVECTOR rayOrigin = XMVector3TransformCoord(XMLoadFloat3(&origin), XMLoadFloat4x4(&inverseEntityMatrix));
	XMVECTOR rayDirection = XMVector3TransformNormal(XMLoadFloat3(&direction), XMLoadFloat4x4(&inverseEntityMatrix));
	rayDirection = XMVector3Normalize(rayDirection);
	XMFLOAT3 rayDirectionF; //this is facing in the right direction! Hooray!
	XMStoreFloat3(&rayDirectionF, rayDirection);
	XMFLOAT3 rayOriginF; //position is half of what the actual camera position is...so *shrugs* worth a shot
	XMStoreFloat3(&rayOriginF, rayOrigin);
	gameEntities[0]->TestPick(cam->GetPosition(), rayDirectionF);//put in camera position instead of calculated one
	//std::cout << rayOriginF.x << " "  << rayOriginF.y << " " << rayOriginF.z << "\n";
	//std::cout << cam->GetPosition().x << " " << cam->GetPosition().y <<" " << cam->GetPosition().z << "\n";
	//std::cout << rayDirectionF.x<< " " << rayDirectionF.y << " " << rayDirectionF.z << "\n";
}
#pragma endregion