#include "GameEntity.h"


using namespace DirectX;
GameEntity::GameEntity(Mesh* pMeshPointer, Material* pMaterial, std::string pName)
{
	this->meshPointer = pMeshPointer;
	this->material = pMaterial;
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);
	box = new BoundingBox();
	box->Center = position;
	box->Extents = meshPointer->getExtents();
	name = pName;

	//load in cube because it's 1x1x1, multiply scale by extents

	/*Vertex vertices[] =
	{
		{ XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.5f, -0.5f, +0.5f), XMFLOAT3(-0.5f, -0.5f, +0.5f) },
		{ XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(+0.5f, -0.5f, +0.5f), XMFLOAT3(+0.5f, -0.5f, +0.5f) },
		{ XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(-0.5f, +0.5f, +0.5f), XMFLOAT3(-0.5f, +0.5f, +0.5f) },
		{ XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(+0.5f, +0.5f, +0.5f) },
		{ XMFLOAT3(-0.5f, +0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(+0.5f, +0.5f, +0.5f) },
		{ XMFLOAT3(+0.5f, +0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(+0.5f, +0.5f, +0.5f) },
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(+0.5f, +0.5f, +0.5f) },
		{ XMFLOAT3(+0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(+0.5f, +0.5f, +0.5f), XMFLOAT3(+0.5f, +0.5f, +0.5f) },
	};
	unsigned indices[] = { 0, 2, 1, 1, 2, 3, };

	debugBox = new Mesh(vertices, 4, indices, 6, device);*/
}


GameEntity::~GameEntity()
{
}

Mesh* GameEntity::GetMesh() {
	return meshPointer;
}

Material* GameEntity::GetMaterial() {
	return material;
}

XMFLOAT4X4 GameEntity::GetWorldMatrix() {
	return worldMatrix;
}

XMFLOAT3 GameEntity::GetPosition() {
	return position;
}

XMFLOAT3 GameEntity::GetRotation() {
	return rotation;
}

XMFLOAT3 GameEntity::GetScale() {
	return scale;
}

BoundingBox* GameEntity::GetBoundingBox() {
	return box;
}

std::string GameEntity::GetName() {
	return name;
}

float GameEntity::TestPick(XMFLOAT3 pOrigin, XMFLOAT3 pDirection) {

	XMStoreFloat3(&(box->Extents), XMVectorMultiply(XMLoadFloat3(&(meshPointer->getExtents())), XMLoadFloat3(&scale)));
	XMStoreFloat3(&(box->Center), XMVectorMultiply(XMLoadFloat3(&(meshPointer->getCenter())), XMLoadFloat3(&scale)));
	XMStoreFloat3(&(box->Center), XMVector3Transform(XMLoadFloat3(&(box->Center)), XMMatrixRotationRollPitchYaw(rotation.x, -rotation.y, rotation.z)));
	XMStoreFloat3(&(box->Center), XMVectorAdd(XMLoadFloat3(&(box->Center)), XMLoadFloat3(&position)));

	//XMStoreFloat3(&(box->Center), XMVector3Transform(XMLoadFloat3(&(meshPointer->getCenter())), XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix))));

	//recalculate bounding box center and extents whenver it changes
	RecalculateBox();

	float dist;
	bool win = box->Intersects(XMLoadFloat3(&pOrigin), XMLoadFloat3(&pDirection), dist);
	if (win) {
		std::cout << name << "\n";
		XMMATRIX inverseWorldMatrix = XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
		XMVECTOR newOrigin = XMVector3Transform(XMLoadFloat3(&pOrigin), inverseWorldMatrix);
		XMVECTOR newDirection = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&pDirection), inverseWorldMatrix));

		//now do triangle hits
		if (meshPointer->TestPick(newOrigin, newDirection)) {
			return dist;
		}
		else {
			//here you didn't hit the mesh
		}
	}
	else {
		//here you didn't hit the box
	}
	return 0;
}


void GameEntity::RecalculateBox() {
	//calculate box to rotate to make extents
	std::vector<XMFLOAT3> points;
	points.push_back(XMFLOAT3(box->Extents.x, box->Extents.y, box->Extents.z));
	points.push_back(XMFLOAT3(-box->Extents.x, box->Extents.y, box->Extents.z));
	points.push_back(XMFLOAT3(-box->Extents.x, -box->Extents.y, box->Extents.z));
	points.push_back(XMFLOAT3(box->Extents.x, -box->Extents.y, box->Extents.z));
	points.push_back(XMFLOAT3(-box->Extents.x, -box->Extents.y, -box->Extents.z));
	points.push_back(XMFLOAT3(-box->Extents.x, box->Extents.y, -box->Extents.z));
	points.push_back(XMFLOAT3(box->Extents.x, -box->Extents.y, -box->Extents.z));
	points.push_back(XMFLOAT3(box->Extents.x, box->Extents.y, -box->Extents.z));

	for (int i = 0; i < points.size(); i++) {
		XMStoreFloat3(&(points[i]), XMVector3Transform(XMLoadFloat3(&(points[i])), XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
	}
	XMFLOAT3 minSize = { 500, 500, 500 };
	XMFLOAT3 maxSize = { -500, -500, -500 };
	for (int i = 0; i < points.size(); i++) {
		if (points[i].x > maxSize.x) maxSize.x = points[i].x;
		if (points[i].x < minSize.x) minSize.x = points[i].x;
		if (points[i].y > maxSize.y) maxSize.y = points[i].y;
		if (points[i].y < minSize.y) minSize.y = points[i].y;
		if (points[i].z > maxSize.z) maxSize.z = points[i].z;
		if (points[i].z < minSize.z) minSize.z = points[i].z;
	}

	XMStoreFloat3(&(box->Extents), XMVectorDivide(XMVectorSubtract(XMLoadFloat3(&maxSize), XMLoadFloat3(&minSize)), XMVectorSet(2.0f, 2.0f, 2.0f, 0.0f)));
}

void GameEntity::SetWorldMatrix(XMFLOAT4X4 pWorldMatrix) {
	worldMatrix = pWorldMatrix;
}

void GameEntity::SetPosition(XMFLOAT3 pPosition) {
	position = pPosition;
}

void GameEntity::SetRotation(XMFLOAT3 pRotation) {
	rotation = pRotation;
}

void GameEntity::SetScale(XMFLOAT3 pScale) {
	scale = pScale;
}

//Adds the given XMFLOAT3 to the current position
void GameEntity::Translate(XMFLOAT3 pTranslate) {
	XMVECTOR newPosition = XMVectorAdd(XMLoadFloat3(&position), XMLoadFloat3(&pTranslate));
	XMStoreFloat3(&position, newPosition);
}

//Adds the given XMFLOAT3 to the current rotation
void GameEntity::Rotate(XMFLOAT3 pRotate) {
	XMVECTOR newRotation = XMVectorAdd(XMLoadFloat3(&rotation), XMLoadFloat3(&pRotate));
	XMStoreFloat3(&rotation, newRotation);
}

//Multiplies the given XMFLOAT3 to the current scale
void GameEntity::Scale(XMFLOAT3 pScale) {
	XMVECTOR newScale = XMVectorMultiply(XMLoadFloat3(&this->scale), XMLoadFloat3(&pScale));
	XMStoreFloat3(&this->scale, newScale);
}

//Calculates the world matrix for this game entity
void GameEntity::CalculateWorldMatrix() {
	XMMATRIX tr = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX ro = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	XMMATRIX sc = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(sc * ro * tr));
}

//Draws mesh and prepares material
void GameEntity::Draw(ID3D11DeviceContext* pContext, Camera* pCam) {

	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* vb = meshPointer->GetVertexBuffer();
	pContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	pContext->IASetIndexBuffer(meshPointer->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	PrepareMaterial(pCam->GetViewMatrix(), pCam->GetProjectionMatrix(), pCam->GetPosition());

	// Finally do the actual drawing
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER
	pContext->DrawIndexed(
		meshPointer->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}

void GameEntity::PrepareMaterial(XMFLOAT4X4 pView, XMFLOAT4X4 pProjection, XMFLOAT3 pCamPosition) {

	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	material->GetVertexShader()->SetMatrix4x4("world", worldMatrix);
	material->GetVertexShader()->SetMatrix4x4("view", pView);
	material->GetVertexShader()->SetMatrix4x4("projection", pProjection);
	material->GetVertexShader()->CopyAllBufferData();

	material->GetPixelShader()->SetFloat3("CameraPosition", pCamPosition);
	material->GetPixelShader()->SetSamplerState("Sampler", material->GetSampler());
	material->GetPixelShader()->SetShaderResourceView("DiffuseTexture", material->GetTexture());
	material->GetPixelShader()->SetShaderResourceView("NormalTexture", material->GetNormal());
	material->GetPixelShader()->CopyAllBufferData();


	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();
}