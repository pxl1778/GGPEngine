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
	XMVECTOR pos = XMVectorSet(0, 0, -16, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!
	XMStoreFloat4x4(&projectionMatrix, XMMatrixIdentity());
	position = XMFLOAT3(0, 0, -14);
	direction = XMFLOAT3(0, 0, 1);
	rotationX = 0;
	rotationY = 0;
	rotationSpeed = 5;
}


Camera::~Camera()
{
	
}

//Freely move camera direction with mouse
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

//Used when looking at something and not being able to shift camera away from target
void Camera::UpdateLookAt(float deltaTime, XMFLOAT3 pTargetLookAt) {
	XMFLOAT4 v = XMFLOAT4();
	XMVECTOR newForward = XMLoadFloat3(&pTargetLookAt) - XMLoadFloat3(&position);
	newForward = XMVector3Normalize(newForward);
	XMVECTOR rightVector = XMVector3Cross(XMVECTOR({ 0, 1, 0 }), XMVector3Normalize(newForward));
	XMVECTOR upVector = XMVector3Cross(newForward, rightVector);
	XMStoreFloat4(&v, upVector);
	if (GetAsyncKeyState('D') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), XMVector3Normalize(rightVector) * rotationSpeed * deltaTime));
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), -XMVector3Normalize(rightVector) * rotationSpeed * deltaTime));
	}
	if (GetAsyncKeyState('W') & 0x8000) {
		XMFLOAT3 rotationOffset;
		XMStoreFloat3(&rotationOffset, XMVector3Normalize(upVector) * rotationSpeed * deltaTime);
		if (position.y + rotationOffset.y <= 13) {
			XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), XMVector3Normalize(upVector) * rotationSpeed * deltaTime));
		}
		else {
			position.y = 13;
		}
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		XMFLOAT3 rotationOffset;
		XMStoreFloat3(&rotationOffset, -XMVector3Normalize(upVector) * rotationSpeed * deltaTime);
		if (position.y + rotationOffset.y >= -11) {
			XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), -XMVector3Normalize(upVector) * rotationSpeed * deltaTime));
		}
		else {
			position.y = -11;
		}
	}
	if (GetAsyncKeyState('Q') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), XMVector3Normalize(newForward) * 3 * deltaTime));
	}
	if (GetAsyncKeyState('E') & 0x8000) {
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), -XMVector3Normalize(newForward) * 3 * deltaTime));
	}
	//XMMATRIX newView = XMMatrixLookAtLH(XMLoadFloat3(&position), XMVector3Normalize(newForward), XMVector3Normalize(upVector));
	XMMATRIX newView = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&pTargetLookAt), XMVector3Normalize(upVector));
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(newView));


	XMMATRIX tr = XMMatrixTranslation(position.x, position.y, position.z);
	//XMMATRIX ro = XMMatrixRotationRollPitchYaw(rotationX, rotationY, rotation.z);

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(tr));
}

void Camera::UpdateProjectionMatrix(unsigned pWidth, unsigned pHeight) {
	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)pWidth / pHeight,		// Aspect ratio
		0.1f,						// Near clip plane distance
		5500.0f);					// Far clip plane distance
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
	//printf("%.2f", rotationX); printf("  "); printf("%.2f", rotationY); printf("\n");
}

XMFLOAT4X4 Camera::GetViewMatrix() {
	return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix() {
	return projectionMatrix;
}

XMFLOAT4X4 Camera::GetWorldMatrix() {
	return worldMatrix;
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

XMVECTOR Camera::GetUpVector(XMFLOAT3 pTargetLookAt) {
	XMVECTOR newForward = XMLoadFloat3(&pTargetLookAt) - XMLoadFloat3(&position);
	newForward = XMVector3Normalize(newForward);
	XMVECTOR rightVector = XMVector3Cross(XMVECTOR({ 0, 1, 0 }), XMVector3Normalize(newForward));
	return XMVector3Normalize(XMVector3Cross(newForward, rightVector));
}

XMVECTOR Camera::GetRightVector(XMFLOAT3 pTargetLookAt) {
	XMVECTOR newForward = XMLoadFloat3(&pTargetLookAt) - XMLoadFloat3(&position);
	newForward = XMVector3Normalize(newForward);
	return XMVector3Normalize(XMVector3Cross(XMVECTOR({ 0, 1, 0 }), XMVector3Normalize(newForward)));
}

XMVECTOR Camera::GetForwardVector(XMFLOAT3 pTargetLookAt) {
	return XMVector3Normalize(XMLoadFloat3(&pTargetLookAt) - XMLoadFloat3(&position));
}

