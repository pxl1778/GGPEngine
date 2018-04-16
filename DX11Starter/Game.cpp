#include "Game.h"
#include "Vertex.h"
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

	delete mat1;
	

	while (!gameEntities.empty()) {
		delete gameEntities.back();
		gameEntities.pop_back();
	}
	gameEntities.clear();

	sampler->Release();
	wallTexture->Release();
	wallNormal->Release();
	skyBoxRastState->Release();
	skyBoxDepthState->Release();

	alphaPostRTV->Release();
	alphaPostSRV->Release();	
	delete alphaPostVertexShader;
	delete alphaPostPixelShader;

	

	delete cam;
	delete guy;
	delete feedButton;

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
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, &alphaPostRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, &alphaPostSRV);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();




	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dLight1 = DirectionalLight({XMFLOAT4(.05f, .05f, .13f, 1.0f), XMFLOAT4(.4f, .3f, .9f, 1.0f), XMFLOAT3(1.0f, -1.0f, 0)});
	dLight2 = DirectionalLight({ XMFLOAT4(0, 0, 0, 0), XMFLOAT4(.01f, .5f, .01f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) });
	pLight1 = PointLight({ XMFLOAT4(1.0f, .1f, .1f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), .1f });

	guy = new Creature(device, context, sampler);




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

	// Load the sky box from a DDS file
	CreateDDSTextureFromFile(device, L"../Assets/Textures/BackgroundPlaceholder.dds", 0, &skyBoxSRV);

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


	//mat1->GetPixelShader()->SetShaderResourceView("");

	feedButton->LoadShaders(device, context, "Oh wait", "I'm trolling");
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
}

void Game::CreateUIButtons()
{
	UIVertex vertices1[] = {
		{ XMFLOAT3(-3.5f, 1.0f, 5.0f), XMFLOAT4(1, 0, 0, 0) },
		{ XMFLOAT3(-3.5f, 1.5f, 5.0f), XMFLOAT4(1, 0, 0, 0) },
		{ XMFLOAT3(-3.0f, 1.0f, 5.0f), XMFLOAT4(1, 0, 0, 0) },
		{ XMFLOAT3(-3.5f, 1.5f, 5.0f), XMFLOAT4(1, 0, 0, 0) },
		{ XMFLOAT3(-3.0f, 1.5f, 5.0f), XMFLOAT4(1, 0, 0, 0) },
		{ XMFLOAT3(-3.0f, 1.0f, 5.0f), XMFLOAT4(1, 0, 0, 0) },
	};

	int indices[] = { 0, 1, 2, 3, 4, 5 };

	feedButton = new UIButton(vertices1, 6, indices, 6, device, 31, 99, 118, 186);
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

	if (GetAsyncKeyState('P') & 0x8000) {
		gs = PAUSE_MENU;
	}

	guy->Update(deltaTime, totalTime);

	for (std::vector<GameEntity*>::iterator it = gameEntities.begin(); it != gameEntities.end(); ++it) {
		(*it)->CalculateWorldMatrix();
	}

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
	context->ClearRenderTargetView(alphaPostRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	context->OMSetRenderTargets(1, &alphaPostRTV, depthStencilView);

	XMFLOAT4 white = XMFLOAT4(1.00, 1.0, 1.0, 1.0);
	
	guy->Draw(context, cam, &dLight1, &dLight2, &pLight1);
	//guy->Draw(context, cam);


	// Render the sky (after all opaque geometry)
	UINT stride = sizeof(Vertex);
	UINT offset = 0;


	ID3D11Buffer* skyVB = m4->GetVertexBuffer();
	ID3D11Buffer* skyIB = m4->GetIndexBuffer();

	// Draw Feed Button
	feedButton->Draw(context, cam->GetProjectionMatrix());


	
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	SkyBoxVertexShader->SetMatrix4x4("view", cam->GetViewMatrix());
	SkyBoxVertexShader->SetMatrix4x4("projection", cam->GetProjectionMatrix());
	SkyBoxVertexShader->CopyAllBufferData();
	SkyBoxVertexShader->SetShader();

	SkyBoxPixelShader->SetShaderResourceView("SkyTexture", skyBoxSRV);
	SkyBoxPixelShader->SetSamplerState("BasicSampler", sampler);
	//SkyBoxPixelShader->CopyAllBufferData();
	SkyBoxPixelShader->SetShader();
	
	context->RSSetState(skyBoxRastState);
	context->OMSetDepthStencilState(skyBoxDepthState, 0);
	int test = m4->GetIndexCount();
	context->DrawIndexed(test, 0, 0);
	
	// At the end of the frame, reset render states
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

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




	//set rendering to back buffer(so stuff actually draws)
	context->OMSetRenderTargets(1, &backBufferRTV, 0);
	//set shaders and associated resources
	alphaPostVertexShader->SetShader();
	alphaPostPixelShader->SetShader();
	alphaPostPixelShader->SetShaderResourceView("Pixels", alphaPostSRV);
	alphaPostPixelShader->SetSamplerState("Sampler", sampler);
	alphaPostPixelShader->SetInt("Bleft", left);
	alphaPostPixelShader->SetInt("Bright", right);
	alphaPostPixelShader->SetInt("Bup", up);
	alphaPostPixelShader->SetInt("Bdown", down);

	alphaPostPixelShader->SetFloat("pixelWidth", 1.0 / width);
	alphaPostPixelShader->SetFloat("pixelHeight", 1.0 / height);
	alphaPostPixelShader->CopyAllBufferData();

	// Unbind vert/index buffers
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw a triangle that will hopefully fill the screen
	context->Draw(3, 0);

	// Unbind this particular register
	alphaPostPixelShader->SetShaderResourceView("Pixels", 0);


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
	printf("x: %d", x);
	printf(", y: %d\n", y);

	// RED FLAG: Hard-coded values atm - YIKES!
	switch (gs) {
		case START_MENU:
			if (x >= button1->x && x <= button1->width && y >= button1->y && y <= button1->height) {
				gs = IN_GAME;
				printf("\nStarting game");
			}

			if (x >= button2->x && x <= button2->width && y >= button2->y && y <= button2->height) {
				Quit();
			}
			break;
		case IN_GAME:
			if(x >= feedButton->x && x <= feedButton->width && y >= feedButton->y && y <= feedButton->height) {
				guy->isFeeding = true;
				printf("\nYou fed the thing");
			}
			break;
		case PAUSE_MENU:
			if (x >= button2->x && x <= button2->width && y >= button2->y && y <= button2->height) {
				gs = IN_GAME;
				printf("\nResuming game");
			}
			break;
		case GAME_OVER:
			if (x >= button3->x && x <= button3->width && y >= button3->y && y <= button3->height) {
				gs = START_MENU;
				printf("\nReturning to Start Menu");
				// don't forget to reset in-game properties here or when transitioning to in-game from start menu
			}
			break;
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

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
#pragma endregion