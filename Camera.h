#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "Transform.h"
#include <DirectXMath.h>

class Camera {
public:
    // Constructor
    Camera(float aspectRatio, DirectX::XMFLOAT3 initialPosition, DirectX::XMFLOAT3 initialRotation = { 0.0f, 0.0f, 0.0f },
        float fov = DirectX::XM_PIDIV4, float nearClip = 0.1f, float farClip = 1000.0f,
        float moveSpeed = 5.0f, float mouseLookSpeed = 0.005f);

    // Getters
    DirectX::XMFLOAT4X4 GetViewMatrix();
    DirectX::XMFLOAT4X4 GetProjectionMatrix();
    Transform& GetTransform();

    // Update methods
    void UpdateProjectionMatrix(float aspectRatio);
    void UpdateViewMatrix();
    void Update(float dt);

private:
    // Fields
    Transform transform; 

    DirectX::XMFLOAT4X4 viewMatrix;        
    DirectX::XMFLOAT4X4 projectionMatrix;  

    float fieldOfView;      
    float nearClipPlane;    
    float farClipPlane;     
    float movementSpeed;    
    float mouseLookSpeed;   

    bool isPerspective;     
};

#endif
