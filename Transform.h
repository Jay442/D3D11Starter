#pragma once
#include <DirectXMath.h>
class Transform
{
public:
    Transform();
    void UpdateMatrices();

    //Setters
    void SetPosition(float x, float y, float z);
    void SetPosition(DirectX::XMFLOAT3 position);
    void SetRotation(float pitch, float yaw, float roll);
    void SetRotation(DirectX::XMFLOAT3 rotation); // XMFLOAT4 for quaternion
    void SetScale(float x, float y, float z);
    void SetScale(DirectX::XMFLOAT3 scale);

    //Getters
    DirectX::XMFLOAT3 GetPosition();
    DirectX::XMFLOAT3 GetPitchYawRoll(); // XMFLOAT4 GetRotation() for quaternion
    DirectX::XMFLOAT3 GetScale();
    DirectX::XMFLOAT4X4 GetWorldMatrix();
    DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

    // Transformers
    void MoveAbsolute(float x, float y, float z);
    void MoveAbsolute(DirectX::XMFLOAT3 offset);
    void Rotate(float pitch, float yaw, float roll);
    void Rotate(DirectX::XMFLOAT3 rotation);
    void Scale(float x, float y, float z);
    void Scale(DirectX::XMFLOAT3 scale);
        
private:
    bool dirty = false;

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 rotation;
    DirectX::XMFLOAT3 scale;

    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 worldInverseTranspose;
    DirectX::XMMATRIX translation;
    DirectX::XMMATRIX scaling;
    DirectX::XMMATRIX rotationPitchYawRoll;
};

