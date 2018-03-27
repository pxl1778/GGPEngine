#include "GameEntity.h"


using namespace DirectX;
GameEntity::GameEntity(Mesh* pMeshPointer, Material* pMaterial)
{
	this->meshPointer = pMeshPointer;
	this->material = pMaterial;
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);
	box = BoundingBox({ {-1, 0, -1}, {1, 2, 1} });
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

void GameEntity::TestPick(XMFLOAT3 pOrigin, XMFLOAT3 pDirection) {
	//https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
	float t1 = (box.min.x - pOrigin.x) / pDirection.x;
	float t2 = (box.max.x - pOrigin.x) / pDirection.x;
	float t3 = (box.min.y - pOrigin.y) / pDirection.y;
	float t4 = (box.max.y - pOrigin.y) / pDirection.y;
	float t5 = (box.min.z - pOrigin.z) / pDirection.z;
	float t6 = (box.max.z - pOrigin.z) / pDirection.z;

	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	if (tmax < 0)
	{
		//return false;
		std::cout << "wrong\n";
		return;
	}
	if (tmin > tmax) {
		//return false;
		std::cout << "wrong\n";
		return;
	}
	std::cout << "hit\n";
	//return true;
	//being weird when rotating camera...
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