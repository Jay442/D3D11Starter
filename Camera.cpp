#include "Camera.h"
#include <DirectXMath.h>
#include  "Input.h"
#include <algorithm>

constexpr float M_PI_2 = 1.57079632679f;

using namespace DirectX;

Camera::Camera(float aspectRatio, XMFLOAT3 initialPosition, XMFLOAT3 initialRotation, float fov, float nearClip, float farClip, float moveSpeed, float mouseLookSpeed)
    : fieldOfView(fov), nearClipPlane(nearClip), farClipPlane(farClip), movementSpeed(moveSpeed), mouseLookSpeed(mouseLookSpeed), isPerspective(true) {
    transform.SetPosition(initialPosition);
    transform.SetRotation(initialRotation);

    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

// Getters
XMFLOAT4X4 Camera::GetViewMatrix() {
    return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix() {
    return projectionMatrix;
}

Transform& Camera::GetTransform() {
    return transform;
}

void Camera::UpdateProjectionMatrix(float aspectRatio) 
{
    XMMATRIX proj = XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearClipPlane, farClipPlane);
    XMStoreFloat4x4(&projectionMatrix, proj);
}

void Camera::UpdateViewMatrix() {
    XMFLOAT3 position = transform.GetPosition();
    XMFLOAT3 forward = transform.GetForward();

    XMVECTOR pos = XMLoadFloat3(&position);
    XMVECTOR dir = XMLoadFloat3(&forward);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookToLH(pos, dir, up);
    XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::Update(float dt) 
{
    if (Input::KeyDown('W')) {
        transform.MoveRelative(0.0f, 0.0f, movementSpeed * dt);
    }
    if (Input::KeyDown('S')) {
        transform.MoveRelative(0.0f, 0.0f, -movementSpeed * dt);
    }
    if (Input::KeyDown('A')) {
        transform.MoveRelative(-movementSpeed * dt, 0.0f, 0.0f);
    }
    if (Input::KeyDown('D')) {
        transform.MoveRelative(movementSpeed * dt, 0.0f, 0.0f);
    }
    if (Input::KeyDown(VK_SPACE)) { 
        transform.MoveRelative(0.0f, movementSpeed * dt, 0.0f);
    }
    if (Input::KeyDown('X')) { 
        transform.MoveRelative(0.0f, -movementSpeed * dt, 0.0f);
    }

    if (Input::MouseRightDown())
    {
        int cursorMovementX = Input::GetMouseXDelta();
        int cursorMovementY = Input::GetMouseYDelta();

        float yaw = cursorMovementX * mouseLookSpeed;
        float pitch = cursorMovementY * mouseLookSpeed;

        transform.Rotate(pitch, yaw, 0.0f);

        float currentPitch = transform.GetPitchYawRoll().x;
        float clampedPitch = std::clamp(currentPitch, -M_PI_2, M_PI_2);
        transform.SetRotation(clampedPitch, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
    }

    UpdateViewMatrix();
}