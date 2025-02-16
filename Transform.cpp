#include "Transform.h"

Transform::Transform()
{
	position = { 0.0f, 0.0f, 0.0f };
	rotation = { 0.0f, 0.0f, 0.0f };
	scale = { 1.0f, 1.0f, 1.0f };
	XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, DirectX::XMMatrixIdentity());

    translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    rotationPitchYawRoll = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    scaling = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
}

void Transform::UpdateMatrices() {
    if (!dirty) return;

    translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
    rotationPitchYawRoll = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    scaling = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

    DirectX::XMMATRIX worldMatrix = scaling * rotationPitchYawRoll * translation;
    XMStoreFloat4x4(&world, worldMatrix);

    DirectX::XMMATRIX worldInverseTransposeMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
    XMStoreFloat4x4(&worldInverseTranspose, worldInverseTransposeMatrix);

    dirty = false;
}

void Transform::SetPosition(float x, float y, float z) {
    position = { x, y, z };
    dirty = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 pos) {
    position = pos;
    dirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll) {
    rotation = { pitch, yaw, roll };
    dirty = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 rot) {
    rotation = rot;
    dirty = true;
}

void Transform::SetScale(float x, float y, float z) {
    scale = { x, y, z };
    dirty = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 scl) {
    scale = scl;
    dirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition() {
    return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll() {
    return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale() {
    return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix() {
    UpdateMatrices();
    return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix() {
    UpdateMatrices();
    return worldInverseTranspose;
}

void Transform::MoveAbsolute(float x, float y, float z) {
    position.x += x;
    position.y += y;
    position.z += z;
    dirty = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset) {
    position.x += offset.x;
    position.y += offset.y;
    position.z += offset.z;
    dirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll) {
    rotation.x += pitch;
    rotation.y += yaw;
    rotation.z += roll;
    dirty = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 rot) {
    rotation.x += rot.x;
    rotation.y += rot.y;
    rotation.z += rot.z;
    dirty = true;
}

void Transform::Scale(float x, float y, float z) {
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;
    dirty = true;
}

void Transform::Scale(DirectX::XMFLOAT3 scl) {
    scale.x *= scl.x;
    scale.y *= scl.y;
    scale.z *= scl.z;
    dirty = true;
}

