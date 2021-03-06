#include "Game.h"
#include "Vertex.h"
#include "DirectXCollision.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
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

	sampler->Release();
	wallTexture->Release();
	wallNormal->Release();
	skyBoxRastState->Release();
	skyBoxDepthState->Release();
	skyBoxSRV->Release();

	delete vertexShader;
	delete pixelShader;
	delete SkyBoxPixelShader;
	delete SkyBoxVertexShader;

	delete m1;
	delete m2;
	delete m3;
	delete m4;
	delete m5; 
	delete m6;	
	delete waterMesh;

	delete mat1;
	delete debugMat;
	delete waterMat;

	while (!gameEntities.empty()) {
		delete gameEntities.back();
		gameEntities.pop_back();
	}
	gameEntities.clear();	

	alphaPostRTV->Release();
	alphaPostSRV->Release();	
	delete alphaPostVertexShader;
	delete alphaPostPixelShader;

	

	while (!debugCubes.empty()) {
		delete debugCubes.back();
		debugCubes.pop_back();
	}
	debugCubes.clear();

	while (!rayEntities.empty()) {
		delete rayEntities.back();
		rayEntities.pop_back();
	}
	rayEntities.clear();

	while (!rayMeshes.empty()) {
		delete rayMeshes.back();
		rayMeshes.pop_back();
	}
	rayMeshes.clear();
	
	delete cam;
	delete guy;

	// delete UI feed button
	//delete feedButton;
	buttonSRV->Release();

	// Clean up refraction resources
	refractSampler->Release();
	refractionRTV->Release();
	refractionSRV->Release();

	//reflectionSRV->Release();

	delete quadVS;
	delete quadPS;
	delete refractVS;
	delete refractPS;

	delete refractionEntity;
	delete refractionMat;
	refractionNormalMap->Release();

	causticLights->projectionTexture->Release();
	delete causticLights;

	//clean up particle stuff
	bubbleTxt->Release();
	heartTxt->Release();
	delete bubbleEmitter;
	delete bubbleEmitter2;
	delete heartEmitter;
	delete particlePS;
	delete particleVS;
	particleDepthState->Release();
	particleBlendState->Release();

	waterBlendState->Release();
	waterNormalMap->Release();

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
	CreateUIButtons();
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();
	// Create a sampler state that holds options for sampling
	// The descriptions should always just be local variables	

	//Set skybox features
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rasterDesc, &skyBoxRastState);

	D3D11_DEPTH_STENCIL_DESC depthScript = {};
	depthScript.DepthEnable = true;
	depthScript.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthScript.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthScript, &skyBoxDepthState);


	//set Post-process effects
	//Alpha pass
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	device->CreateTexture2D(&textureDesc, 0, &ppTexture);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC JPrtvDesc = {};
	JPrtvDesc.Format = textureDesc.Format;
	JPrtvDesc.Texture2D.MipSlice = 0;
	JPrtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &JPrtvDesc, &alphaPostRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC JPsrvDesc = {};
	JPsrvDesc.Format = textureDesc.Format;
	JPsrvDesc.Texture2D.MipLevels = 1;
	JPsrvDesc.Texture2D.MostDetailedMip = 0;
	JPsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &JPsrvDesc, &alphaPostSRV);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();




	// Refraction setup ------------------------
	ID3D11Texture2D* refractionRenderTexture;
	
	// Set up render texture
	D3D11_TEXTURE2D_DESC rtDesc = {};
	rtDesc.Width = width;
	rtDesc.Height = height;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtDesc.Usage = D3D11_USAGE_DEFAULT;
	rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rtDesc.CPUAccessFlags = 0;
	rtDesc.MiscFlags = 0;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.SampleDesc.Quality = 0;
	device->CreateTexture2D(&rtDesc, 0, &refractionRenderTexture);


	// Set up render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = rtDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(refractionRenderTexture, &rtvDesc, &refractionRTV);

	// Set up shader resource view for same texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(refractionRenderTexture, &srvDesc, &refractionSRV);

	// All done with this texture ref
	refractionRenderTexture->Release();

	// Set up a sampler that uses clamp addressing
	// for use when doing refration - this is useful so 
	// that we don't wrap the refraction from the other
	// side of the screen
	D3D11_SAMPLER_DESC rSamp = {};
	rSamp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	rSamp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	rSamp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	rSamp.Filter = D3D11_FILTER_ANISOTROPIC;
	rSamp.MaxAnisotropy = 16;
	rSamp.MaxLOD = D3D11_FLOAT32_MAX;

	// Ask DirectX for the actual object
	device->CreateSamplerState(&rSamp, &refractSampler);


	//particle setup--------------------------------------------------------
	//load texture
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/bubble.png", 0, &bubbleTxt);
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/heart.png", 0, &heartTxt);

	//depth state for particles
	D3D11_DEPTH_STENCIL_DESC pDesc = {};
	pDesc.DepthEnable = true;
	pDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //turns off depth writing
	pDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&pDesc, &particleDepthState);

	//blend for particles
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, &particleBlendState);

	D3D11_BLEND_DESC waterBlendDesc = {};
	waterBlendDesc.AlphaToCoverageEnable = false;
	waterBlendDesc.IndependentBlendEnable = false;
	waterBlendDesc.RenderTarget[0].BlendEnable = true;
	waterBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	waterBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	waterBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	waterBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	waterBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	waterBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	waterBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&waterBlendDesc, &waterBlendState);

	// Set up particles
	bubbleEmitter = new Emitter(
		11,							// Max particles
		.8f,							// Particles per second
		20,								// Particle lifetime
		0.1f,							// Start size
		2.0f,							// End size
		XMFLOAT4(1, 1, 1, .5f),	// Start color
		XMFLOAT4(.7f, 1, 0.7f, 0),		// End color
		XMFLOAT3(0, .15f, 0),				// Start velocity
		XMFLOAT3(4, -2.5f, 0),				// Start position
		XMFLOAT3(0, .1f, 0),				// Start acceleration
		device,
		particleVS,
		particlePS,
		bubbleTxt);

	bubbleEmitter2 = new Emitter(
		11,							// Max particles
		.8f,							// Particles per second
		20,								// Particle lifetime
		0.1f,							// Start size
		2.0f,							// End size
		XMFLOAT4(1, 1, 1, .5f),	// Start color
		XMFLOAT4(.7f, 1, 0.7f, 0),		// End color
		XMFLOAT3(0, .15f, 0),				// Start velocity
		XMFLOAT3(-4, -2.5f, 0),				// Start position
		XMFLOAT3(0, .1f, 0),				// Start acceleration
		device,
		particleVS,
		particlePS,
		bubbleTxt);

	heartEmitter = new Emitter(
		5,							// Max particles
		1.0f,							// Particles per second
		3,								// Particle lifetime
		1.0f,							// Start size
		2.0f,							// End size
		XMFLOAT4(1, 1, 1, 1),	// Start color
		XMFLOAT4(1,1,1,.5),		// End color
		XMFLOAT3(0, .3f, 0),				// Start velocity
		XMFLOAT3(0, 4.5, 0),				// Start position
		XMFLOAT3(0, 1.0f, 0),				// Start acceleration
		device,
		particleVS,
		particlePS,
		heartTxt);


	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dLight1 = DirectionalLight({XMFLOAT4(.05f, .05f, .13f, 1.0f), XMFLOAT4(0.0f, .95f, 1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0)});
	dLight2 = DirectionalLight({ XMFLOAT4(0, 0, 0, 0), XMFLOAT4(.01f, .5f, .01f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	pLight1 = PointLight({ XMFLOAT4(1.0f, .1f, .1f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), .1f });

	guy = new Creature(device, context, sampler);

	for (std::vector<GameEntity*>::iterator it = guy->gameEntities.begin(); it != guy->gameEntities.end(); ++it) {
		debugCubes.push_back(new GameEntity(m4, debugMat, "debugcube"));
	}

	/*gameEntities.push_back(new GameEntity(waterMesh, waterMat, "water"));
	gameEntities.back()->SetRotation(XMFLOAT3(XM_PI, 0, 0));
	gameEntities.back()->Translate(XMFLOAT3(0, 1200.0f, 0));
	gameEntities.back()->Scale(XMFLOAT3(500.0f, 1.0f, 500.0f));*/

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
	//CreateWICTextureFromFile(device, context, L"../Assets/Textures/angryeyes_reflection.tif", 0, &reflectionSRV);

	// Load the sky box from a DDS file
	CreateDDSTextureFromFile(device, L"../Assets/Textures/WhirlpoolSkybox.dds", 0, &skyBoxSRV);

	D3D11_SAMPLER_DESC sd = {};
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sd, &sampler);


	//Material 1
	mat1 = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), wallTexture, wallNormal, sampler);
	debugMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), wallTexture, wallNormal, sampler);
	debugMat->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	debugMat->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
	mat1->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	mat1->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");


	//set up projection of caustic lights-----------------------------------
	causticLights = new Projection({ XMFLOAT4X4(), XMFLOAT4X4(),nullptr });
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		.75f * 3.1415926535f,		// Field of View Angle
		256 / 256,					// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&causticLights->projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
	XMFLOAT3 position = XMFLOAT3(0, 1, 0);
	XMVECTOR forward = XMVectorSet(.01f, -1.0f, 0.0f, 0.0f);
	XMVECTOR right = XMVector3Cross(XMVECTOR({ 0, 1, 0 }), XMVector3Normalize(forward));
	XMVECTOR up = XMVector3Cross(forward, right);
	XMMATRIX newView = XMMatrixLookToLH(XMLoadFloat3(&position), XMVector3Normalize(forward), XMVector3Normalize(up));
	XMStoreFloat4x4(&causticLights->viewMatrix, XMMatrixTranspose(newView));
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/caustic.png", 0, &causticLights->projectionTexture);

	//Water Material
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/waternormal.jpg", 0, &waterNormalMap);
	waterMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), wallTexture, waterNormalMap, sampler);
	waterMat->GetVertexShader()->LoadShaderFile(L"WaterVertexShader.cso");
	waterMat->GetPixelShader()->LoadShaderFile(L"WaterPixelShader.cso");
	//skybox loading
	SkyBoxVertexShader = new SimpleVertexShader(device, context);
	SkyBoxPixelShader = new SimplePixelShader(device, context);
	SkyBoxVertexShader->LoadShaderFile(L"SkyBoxVertexShader.cso");
	SkyBoxPixelShader->LoadShaderFile(L"SkyBoxPixelShader.cso");

	//Post Process Loading
	alphaPostVertexShader = new SimpleVertexShader(device, context);
	alphaPostPixelShader = new SimplePixelShader(device, context);
	alphaPostVertexShader->LoadShaderFile(L"BlurVertexShader.cso");
	alphaPostPixelShader->LoadShaderFile(L"BlurPixelShader.cso");

	//particle shader loading
	particleVS = new SimpleVertexShader(device, context);
	particleVS->LoadShaderFile(L"ParticleVS.cso");
	particlePS = new SimplePixelShader(device, context);
	particlePS->LoadShaderFile(L"ParticlePS.cso");

	//mat1->GetPixelShader()->SetShaderResourceView("");
	// UI button shader
	//feedButton->LoadShaders(device, context, "Oh wait", "I'm trolling");

	// Refraction shaders
	quadVS = new SimpleVertexShader(device, context);
	quadVS->LoadShaderFile(L"FullscreenQuadVS.cso");

	quadPS = new SimplePixelShader(device, context);
	quadPS->LoadShaderFile(L"FullscreenQuadPS.cso");

	refractVS = new SimpleVertexShader(device, context);
	refractVS->LoadShaderFile(L"RefractVS.cso");

	refractPS = new SimplePixelShader(device, context);
	refractPS->LoadShaderFile(L"RefractPS.cso");

	// load in normal map for refraction object
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/water_normal.png", 0, &refractionNormalMap);

	// create the material of refraction object
	refractionMat = new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context), 0, refractionNormalMap, sampler);
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
	waterMesh = new Mesh("../Assets/Models/50x50.obj", device);


	// Set up the refraction entity (the object that refracts)
	refractionEntity = new GameEntity(m1, refractionMat, "refractionBall"); 
	refractionEntity->SetPosition(XMFLOAT3(0, 1, 0));
	refractionEntity->SetScale(XMFLOAT3(20, 20, 20)); 
}

void Game::CreateUIButtons()
{
	//UIVertex vertices1[] = {
	//	{ XMFLOAT3(-3.5f, 1.0f, 5.0f), XMFLOAT4(1, 0, 0, 0) }, // Bottom-Left
	//	{ XMFLOAT3(-3.5f, 1.5f, 5.0f), XMFLOAT4(1, 0, 0, 0) }, // Top-Left
	//	{ XMFLOAT3(-3.0f, 1.0f, 5.0f), XMFLOAT4(1, 0, 0, 0) }, // Bottom-Right
	//	{ XMFLOAT3(-3.0f, 1.5f, 5.0f), XMFLOAT4(1, 0, 0, 0) }, // Top-Right
	//};
	//
	//int indices[] = { 0, 1, 2, 3 };

	//feedButton = new UIButton(vertices1, 4, indices, 4, device, 31, 99, 118, 186);
	//feedButton = new UIButton(vertices1, 4, indices, 4, device, width/2 - width/27 - width/2.27, height/2 - height/16 - height/3.3, width/2 + width/27 - width/2.25, height/2 + height/16 - height/3.3);

	feedButton2 = { 50, 50, 150, 150 };
	CreateWICTextureFromFile(device, context, L"../Assets/Textures/feedButton.png", 0, &buttonSRV);
}

void Game::DrawScene()
{
	guy->Draw(context, cam, &dLight1, &dLight2, &pLight1, skyBoxSRV, causticLights);
}

void Game::DrawSky()
{
	ID3D11Buffer* skyVB = m4->GetVertexBuffer();
	ID3D11Buffer* skyIB = m4->GetIndexBuffer();

	// Set the buffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	// Set up the sky shaders
	SkyBoxVertexShader->SetMatrix4x4("view", cam->GetViewMatrix());
	SkyBoxVertexShader->SetMatrix4x4("projection", cam->GetProjectionMatrix());
	SkyBoxVertexShader->CopyAllBufferData();
	SkyBoxVertexShader->SetShader();


	SkyBoxPixelShader->SetShaderResourceView("SkyTexture", skyBoxSRV);
	SkyBoxPixelShader->SetSamplerState("BasicSampler", sampler);
	SkyBoxPixelShader->SetShader();

	// Set up the render state options
	context->RSSetState(skyBoxRastState);
	context->OMSetDepthStencilState(skyBoxDepthState, 0);

	// Do the actual drawing 
	int test = m4->GetIndexCount();
	context->DrawIndexed(test, 0, 0);

	// At the end of the frame, reset render states
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

void Game::DrawRefraction()
{
	ID3D11Buffer* vb = refractionEntity->GetMesh()->GetVertexBuffer();
	ID3D11Buffer* ib = refractionEntity->GetMesh()->GetIndexBuffer();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	// Setup vertex shader
	refractVS->SetMatrix4x4("world", refractionEntity->GetWorldMatrix());
	refractVS->SetMatrix4x4("view", cam->GetViewMatrix());
	refractVS->SetMatrix4x4("projection", cam->GetProjectionMatrix());
	refractVS->CopyAllBufferData();
	refractVS->SetShader();

	// Setup pixel shader
	refractPS->SetShaderResourceView("ScenePixels", refractionSRV);	// Pixels of the screen
	refractPS->SetShaderResourceView("NormalMap", refractionNormalMap);	// Normal map for the object itself
	refractPS->SetSamplerState("BasicSampler", sampler);			// Sampler for the normal map
	refractPS->SetSamplerState("RefractSampler", refractSampler);	// Uses CLAMP on the edges
	refractPS->SetFloat3("CameraPosition", cam->GetPosition());
	refractPS->SetMatrix4x4("view", cam->GetViewMatrix());				// View matrix, so we can put normals into view space
	refractPS->CopyAllBufferData();
	refractPS->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(refractionEntity->GetMesh()->GetIndexCount(), 0, 0);
}

void Game::DrawBlurEffect()
{
	//Switch to post Process mode
	//First Pass
	int left = 0, right = 1, down = 0, up = 1;
	if (GetAsyncKeyState('Q') & 0x8000) {
		down = -5;
	}
	if (GetAsyncKeyState('E') & 0x8000) {
		up = 5;
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		right = 5;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		left = -5;
	}

	alphaPostVertexShader->SetShader();
	alphaPostPixelShader->SetShader();
	alphaPostPixelShader->SetShaderResourceView("Pixels", refractionSRV);
	alphaPostPixelShader->SetSamplerState("Sampler", sampler);
	alphaPostPixelShader->SetInt("Bleft", left);
	alphaPostPixelShader->SetInt("Bright", right);
	alphaPostPixelShader->SetInt("Bup", up);
	alphaPostPixelShader->SetInt("Bdown", down);

	alphaPostPixelShader->SetFloat("pixelWidth", 1.0f / width);
	alphaPostPixelShader->SetFloat("pixelHeight", 1.0f / height);
	alphaPostPixelShader->CopyAllBufferData();

	// Unbind vert/index buffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw a triangle that will hopefully fill the screen
	context->Draw(3, 0);

	// Unbind this particular register
	alphaPostPixelShader->SetShaderResourceView("Pixels", 0);
}

void Game::DrawFullscreenQuad(ID3D11ShaderResourceView * texture)
{
	// First, turn off our buffers, as we'll be generating the vertex
	// data on the fly in a special vertex shader using the index of each vert
	context->IASetVertexBuffers(0, 0, 0, 0, 0);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Set up the fullscreen quad shaders
	quadVS->SetShader();

	quadPS->SetShaderResourceView("Pixels", texture);
	quadPS->SetSamplerState("Sampler", sampler);
	quadPS->SetShader();

	// Draw
	context->Draw(3, 0);
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

	//feedButton->x = width / 2 - width / 27 - width / 3;
	//feedButton->width = width / 2 + width / 27 - width / 3;
	//feedButton->y = height / 2 - height / 16 - height / 3.3;
	//feedButton->height = height / 2 + height / 16 - height / 3.3;
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	if (GetAsyncKeyState('P') & 0x8000) {
		gs = PAUSE_MENU;
	}

	guy->Update(deltaTime, totalTime);
	//Debug Cubes
	if (debugMode) {
		for (int i = 0; i < guy->gameEntities.size(); i++) {
			XMVECTOR newExtents = XMLoadFloat3(&(guy->gameEntities[i]->GetBoundingBox()->Extents)) * 2;
			XMFLOAT3 nE;
			XMStoreFloat3(&nE, newExtents);
			debugCubes[i]->SetScale(nE);
			debugCubes[i]->SetPosition(guy->gameEntities[i]->GetBoundingBox()->Center);
		}
		for (std::vector<GameEntity*>::iterator it = debugCubes.begin(); it != debugCubes.end(); ++it) {
			(*it)->CalculateWorldMatrix();
		}
		for (std::vector<GameEntity*>::iterator it = rayEntities.begin(); it != rayEntities.end(); ++it) {
			(*it)->CalculateWorldMatrix();
		}
	}

	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->CalculateWorldMatrix();
	}

	cam->UpdateLookAt(deltaTime, XMFLOAT3(0, 1, 0)); //Here is where we'd pass in the creature's position
	pLight1.Position = XMFLOAT3(pLight1.Position.x, sin(totalTime) * .5f, pLight1.Position.z);

	refractionEntity->Rotate(XMFLOAT3(0, deltaTime * 0.05f, 0));
	refractionEntity->CalculateWorldMatrix();

	bubbleEmitter->Update(deltaTime);
	bubbleEmitter2->Update(deltaTime);
	if (guy->guyState == Happy) { heartEmitter->Update(deltaTime); }
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
	context->ClearRenderTargetView(refractionRTV, color);
	context->ClearRenderTargetView(alphaPostRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	// Use our refraction render target and our regular depth buffer
	context->OMSetRenderTargets(1, &refractionRTV, depthStencilView);

	XMFLOAT4 white = XMFLOAT4(1.00, 1.0, 1.0, 1.0);



	if (debugMode) {
		for (std::vector<GameEntity*>::iterator it = debugCubes.begin(); it != debugCubes.end(); ++it) {
			(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
			(*it)->GetMaterial()->GetVertexShader()->SetData("color", &white, sizeof(XMFLOAT4));
			(*it)->Draw(context, cam);
		}
		for (std::vector<GameEntity*>::iterator it = rayEntities.begin(); it != rayEntities.end(); ++it) {
			(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
			(*it)->GetMaterial()->GetVertexShader()->SetData("color", &white, sizeof(XMFLOAT4));
			(*it)->Draw(context, cam);
		}
	}

	// Draw the scene (WITHOUT the refracting object)
	DrawScene();

	DrawSky();


	context->OMSetBlendState(waterBlendState, 0, 0xffffffff);
	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetVertexShader()->SetData("color", &white, sizeof(XMFLOAT4));
		(*it)->Draw(context, cam);
	}
	context->OMSetBlendState(0, 0, 0xffffffff);
	
	float blend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, blend, 0xffffffff);
	context->OMSetDepthStencilState(particleDepthState, 0);

	bubbleEmitter->Draw(context, cam);
	bubbleEmitter2->Draw(context, cam);
	if (guy->guyState == Happy) { heartEmitter->Draw(context, cam); }

	// reset to default states
	context->OMSetBlendState(0, blend, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);

	// Back to the screen, but NO depth buffer for now!
	// We just need to plaster the pixels from the render target onto the 
	// screen without affecting (or respecting) the existing depth buffer
	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	// Do blur effect
	DrawBlurEffect(); // can't use DrawFullscreenQuad() fxn with blur effect D:

	// Turn the depth buffer back on, so we can still
	// used the depths from our earlier scene render
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	// Draw the refraction object
	DrawRefraction();

	// Unbind all textures at the end of the frame
	// This is a good idea any time we're using extra render targets
	// that we intend to sample from on the next frame
	ID3D11ShaderResourceView* nullSRV[16] = {};
	context->PSSetShaderResources(0, 16, nullSRV);

	// Draw Feed Button
	spriteBatch->Begin();
	spriteBatch->Draw(buttonSRV, feedButton2);
	spriteBatch->End();

	// Don't forget to reset states!
	float blendFactors[4] = { 1,1,1,1 };
	context->OMSetBlendState(0, blendFactors, 0xFFFFFFFF);
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

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
	//printf("x: %d", x);
	//printf(", y: %d\n", y);

	// RED FLAG: Hard-coded values atm - YIKES!
	switch (gs) {
		case START_MENU:
			if (x >= startButton.left && x <= startButton.right && y >= startButton.top && y <= startButton.bottom) {
				gs = IN_GAME;
				printf("\nStarting game");
			}

			if (x >= exitButton.left && x <= exitButton.right && y >= exitButton.top && y <= exitButton.bottom) {
				Quit();
			}
			break;
		case IN_GAME:
			if (x >= feedButton2.left && x <= feedButton2.right && y >= feedButton2.top && y <= feedButton2.bottom) {
				guy->isFeeding = true;
				printf("\nYou fed the thing");
			}

			break;
		case PAUSE_MENU:
			if (x >= resumeButton.left && x <= resumeButton.right && y >= resumeButton.top && y <= resumeButton.bottom) {
				gs = IN_GAME;
				printf("\nResuming game");
			}
			break;
		case GAME_OVER:
			if (x >= returnButton.left && x <= returnButton.right && y >= returnButton.top && y <= returnButton.bottom) {
				gs = START_MENU;
				printf("\nReturning to Start Menu");
				// don't forget to reset in-game properties here or when transitioning to in-game from start menu
			}
			break;
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	//TestInteraction(x, y);

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
	guy->guyState = Neutral;
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
		TestInteraction(x, y);
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
	//https://code.msdn.microsoft.com/windowsapps/How-to-pick-and-manipulate-089639ab/sourcecode?fileId=124643&pathId=1248898311
	float pointX = (((2.0f * (float)pMouseX) / (float)width) - 1.0f) / cam->GetProjectionMatrix()._11;
	float pointY = ((((2.0f * (float)pMouseY) / (float)height) - 1.0f)) / cam->GetProjectionMatrix()._22;

	XMVECTOR camPos = XMLoadFloat3(&(cam->GetPosition()));

	//chris's method (Turns out the camera's view matrix is already inverted
	XMVECTOR rayVS = XMVectorSet(pointX, pointY * -1, 1, 0);
	XMVECTOR rayWS = XMVector3TransformNormal(rayVS, XMLoadFloat4x4(&(cam->GetViewMatrix())));
	rayWS = XMVector3Normalize(rayWS);

	XMFLOAT3 rayDirectionF;
	XMStoreFloat3(&rayDirectionF, rayWS);
	XMFLOAT3 rayOriginF;
	XMStoreFloat3(&rayOriginF, camPos);

	////////////////////////////////////////////////////////DEBUG RAYS
	if (debugMode) {
		XMMATRIX inverseWorldMatrix = XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&guy->gameEntities[6]->GetWorldMatrix())));
		XMVECTOR newOrigin = XMVector3Transform(XMLoadFloat3(&rayOriginF), inverseWorldMatrix);
		XMVECTOR newDirection = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&rayDirectionF), inverseWorldMatrix));
		XMFLOAT3 debugOrigin1;
		XMFLOAT3 debugOrigin2;
		XMFLOAT3 debugEnd1;
		XMFLOAT3 debugEnd2;
		/*XMStoreFloat3(&debugOrigin1, XMLoadFloat3(&rayOriginF) + (cam->GetUpVector(XMFLOAT3(0, 0, 0))*.1));
		XMStoreFloat3(&debugOrigin2, XMLoadFloat3(&rayOriginF) - (cam->GetUpVector(XMFLOAT3(0, 0, 0))*.1));
		XMStoreFloat3(&debugEnd1, (XMLoadFloat3(&rayDirectionF) * 100) + XMLoadFloat3(&debugOrigin1));
		XMStoreFloat3(&debugEnd2, (XMLoadFloat3(&rayDirectionF) * 100) + XMLoadFloat3(&debugOrigin2));*/
		XMStoreFloat3(&debugOrigin1, newOrigin + (cam->GetUpVector(XMFLOAT3(0.0f, 0.0f, 0.0f))*.1f));
		XMStoreFloat3(&debugOrigin2, newOrigin - (cam->GetUpVector(XMFLOAT3(0.0f, 0.0f, 0.0f))*.1f));
		XMStoreFloat3(&debugEnd1, (newDirection * 100) + XMLoadFloat3(&debugOrigin1));
		XMStoreFloat3(&debugEnd2, (newDirection * 100) + XMLoadFloat3(&debugOrigin2));
		Vertex vertices[] =
		{
			{ debugEnd2, XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ debugOrigin2, XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ debugEnd1, XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
			{ debugOrigin1, XMFLOAT2(1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};
		unsigned indices[] = { 0, 2, 1, 1, 2, 3 };

		rayMeshes.push_back(new Mesh(vertices, 4, indices, 6, device));
		rayEntities.push_back(new GameEntity(rayMeshes.back(), debugMat, "ray"));
	}
	////////////////////////////////////////////////////////////////

	float minDistance = 1000; 
	float currentDistance = 0;
	GameEntity* closestEntity = nullptr;
	for (int i = 0; i < guy->gameEntities.size(); i++) {
		currentDistance = guy->gameEntities[i]->TestPick(rayOriginF, rayDirectionF);
		if (currentDistance > 0) {
			//std::cout << guy->gameEntities[i]->GetName() << "\n";
		}
		if (currentDistance > 0 && currentDistance < minDistance) {
			minDistance = currentDistance;
			closestEntity = guy->gameEntities[i];
		}
	}
	if (closestEntity == nullptr) {
		//std::cout << "No hit\n";
		guy->guyState = Neutral;
	}
	else {
		
		//check if clicking body
		if (closestEntity->GetName().compare("body") == 0) {
			guy->guyState = Happy;
		}
		//check if clicking tentacles
		else if (closestEntity->GetName().find("tentacle") != std::string::npos) {
			guy->guyState = Angry;
		}
	}
}
#pragma endregion