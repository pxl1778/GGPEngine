#include "Camera.h"


using namespace DirectX;
Camera::Camera()
{
	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!
	XMStoreFloat4x4(&projectionMatrix, XMMatrixIdentity());
	position = XMFLOAT3(0, 0, -5);
	direction = XMFLOAT3(0, 0, 1);
	rotationX = 0;
	rotationY = 0;
}


Camera::~Camera()
{
	
}

void Camera::Update(float deltaTime) {
	XMVECTOR rotationVector = XMQuaternionRotationRollPitchYaw(rotationY, rotationX, 0);
	XMFLOAT4 v = XMFLOAT4();
	XMVECTOR newForward = XMVector3Rotate(XMLoadFloat3(&direction), rotationVector);
	XMVECTOR rightVector = XMVector3Cross(XMVECTOR({ 0, 1, 0 }), XMVector3Normalize(newForward));
	XMVECTOR upVector = XMVector3Cross(newForward, rightVector);
	XMStoreFloat4(&v, upVector);
	if (GetAsyncKeyState('W') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), XMVector3Normalize(newForward) * 3 *deltaTime));
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), -XMVector3Normalize(newForward) * 3 *deltaTime));
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), XMVector3Normalize(rightVector) * 3 *deltaTime));
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), -XMVector3Normalize(rightVector) * 3 *deltaTime));
	}
	XMMATRIX newView = XMMatrixLookToLH(XMLoadFloat3(&position), XMVector3Normalize(newForward), XMVector3Normalize(upVector));
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(newView));
}

void Camera::UpdateProjectionMatrix(unsigned pWidth, unsigned pHeight) {
	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)pWidth / pHeight,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

//Updates the rotationX and rotationY when the mouse moves
void Camera::UpdateRotation(float pX, float pY) {
	if (pY > .015f) {
		pY = .015f;
	}
	if (pY < -.015f) {
		pY = -.015f;
	}
	if (pX > .015f) {
		pX = .015f;
	}
	if (pX < -.015f) {
		pX = -.015f;
	}
	rotationX += pX;
	rotationY += pY;
	/*if (rotationY > 3.14) {
		rotationY = 0;
	}
	if (rotationY < 0) {
		rotationY = 3.14;
	}
	if (rotationX > 3.14) {
		rotationX = 0;
	}
	if (rotationX < 0) {
		rotationX = 3.14;
	}*/
	printf("%.2f", rotationX); printf("  "); printf("%.2f", rotationY); printf("\n");
}

XMFLOAT4X4 Camera::GetViewMatrix() {
	return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix() {
	return projectionMatrix;
}

XMFLOAT3 Camera::GetPosition() {
	return position;
}

XMFLOAT3 Camera::GetDirection() {
	return direction;
}

float Camera::GetRotationX() {
	return rotationX;
}

float Camera::GetRotationY() {
	return rotationY;
}

