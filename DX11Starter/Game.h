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
#include "Creature.h"
#include "UIButton.h"


class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);
private:

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();
	
	void TestInteraction(int pMouseX, int pMouseY);
	void CreateUIButtons();

	// Render helper methods
	void DrawScene();
	void DrawSky();
	void DrawRefraction();
	void DrawFullscreenQuad(ID3D11ShaderResourceView* texture);

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimpleVertexShader* SkyBoxVertexShader;

	//Skybox Stuff
	SimplePixelShader* pixelShader;
	SimplePixelShader* SkyBoxPixelShader;
	ID3D11RasterizerState* skyBoxRastState;
	ID3D11DepthStencilState* skyBoxDepthState;
	ID3D11ShaderResourceView* skyBoxSRV;

	//Post-Process stuff
	ID3D11RenderTargetView* alphaPostRTV;		// Allows us to render to a texture
	ID3D11ShaderResourceView* alphaPostSRV;	// Allows us to sample from the same texture
	SimpleVertexShader* alphaPostVertexShader;
	SimplePixelShader* alphaPostPixelShader;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	//Meshes
	Mesh* m1;
	Mesh* m2;
	Mesh* m3;
	Mesh* m4;
	Mesh* m5;
	Mesh* m6;

	std::vector<Mesh*> rayMeshes;

	//Materials
	Material* mat1;
	Material* debugMat;
	Material* SkyBoxMat;

	//Texture Stuff
	ID3D11ShaderResourceView* wallTexture;
	ID3D11ShaderResourceView* wallNormal;
	ID3D11SamplerState* sampler;

	//Entities
	std::vector<GameEntity*> gameEntities;
	std::vector<GameEntity*> debugCubes;
	std::vector<GameEntity*> rayEntities;
	//Camera
	Camera* cam;

	//Lights
	DirectionalLight dLight1;
	DirectionalLight dLight2;
	PointLight pLight1;

	//main character
	Creature* guy;

	// UI Button
	UIButton* feedButton;

	bool debugMode = false;

	// Refraction-related variables
	ID3D11SamplerState* refractSampler;
	ID3D11RenderTargetView* refractionRTV;
	ID3D11ShaderResourceView* refractionSRV;
	SimpleVertexShader* refractVS;
	SimplePixelShader* refractPS;
	SimpleVertexShader* quadVS;
	SimplePixelShader* quadPS;

	GameEntity* refractionEntity;
	Material* refractionMat;
	ID3D11ShaderResourceView* refractionNormalMap;
};

