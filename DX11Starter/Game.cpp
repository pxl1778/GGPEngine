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

	delete m1;
	delete m2;
	delete m3;
	delete m4;
	delete m5; 
	delete m6;


	
	delete SkyBoxPixelShader;
	delete SkyBoxVertexShader;

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
	

	delete cam;

	delete guy;
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
	

	

	// Create a sampler state that holds options for sampling
	// The descriptions should always just be local variables	

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rasterDesc, &skyBoxRastState);

	D3D11_DEPTH_STENCIL_DESC depthScript = {};
	depthScript.DepthEnable = true;
	depthScript.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthScript.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthScript, &skyBoxDepthState);


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
	SkyBoxVertexShader = new SimpleVertexShader(device, context);
	SkyBoxPixelShader = new SimplePixelShader(device, context);

	

	SkyBoxVertexShader->LoadShaderFile(L"SkyBoxVertexShader.cso");
	SkyBoxPixelShader->LoadShaderFile(L"SkyBoxPixelShader.cso");


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


	

	//gameEntities.push_back(new GameEntity(m1, mat1));
	//gameEntities[0]->SetPosition(XMFLOAT3(.7f, 0, 0));
	//gameEntities.push_back(new GameEntity(m2, mat1));
	//gameEntities[1]->SetPosition(XMFLOAT3(-.7f, 0, 0));
	//gameEntities.push_back(new GameEntity(m3, mat1));
	//gameEntities[2]->SetPosition(XMFLOAT3(0, 0, -1));

	
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

	guy->Update(deltaTime, totalTime);

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

	XMFLOAT4 white = XMFLOAT4(1.00, 1.0, 1.0, 1.0);

	/*for (std::vector<GameEntity*>::iterator it = guy->gameEntities.begin(); it != guy->gameEntities.end(); ++it) {
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight1", &dLight1, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetPixelShader()->SetData("dLight2", &dLight2, sizeof(DirectionalLight));
		(*it)->GetMaterial()->GetPixelShader()->SetData("pLight1", &pLight1, sizeof(PointLight));
		(*it)->GetMaterial()->GetVertexShader()->SetData("color", &white, sizeof(XMFLOAT4));
		(*it)->Draw(context, cam);
	}*/

	guy->Draw(context, cam, &dLight1, &dLight2, &pLight1);
	//guy->Draw(context, cam);

	// Render the sky (after all opaque geometry)
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	ID3D11Buffer* skyVB = m4->GetVertexBuffer();
	ID3D11Buffer* skyIB = m4->GetIndexBuffer();
	
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