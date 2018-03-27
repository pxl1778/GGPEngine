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

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimpleVertexShader* SkyBoxVertexShader;
	SimplePixelShader* pixelShader;
	SimplePixelShader* SkyBoxPixelShader;
	

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
	Mesh* Box;

	

	//Materials
	Material* mat1;
	Material* SkyBoxMat;
	

	//Texture Stuff
	ID3D11ShaderResourceView* wallTexture;
	ID3D11ShaderResourceView* wallNormal;
	ID3D11ShaderResourceView* skyBoxSRV;
	ID3D11SamplerState* sampler;


	ID3D11RasterizerState* skyBoxRastState;
	ID3D11DepthStencilState* skyBoxDepthState;
	//Entities
	std::vector<GameEntity*> gameEntities;

	//Camera
	Camera* cam;

	//Lights
	DirectionalLight dLight1;
	DirectionalLight dLight2;
	PointLight pLight1;

	//main character
	Creature* guy;
};

