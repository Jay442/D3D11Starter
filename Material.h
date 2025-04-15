#pragma once
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>
#include "Transform.h"
#include "Camera.h"

class Material
{
public:
    // Constructor
    Material(
        DirectX::XMFLOAT4 colorTint,
        std::shared_ptr<SimpleVertexShader> vs,
        std::shared_ptr<SimplePixelShader> ps,
        DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1),
        DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));

    // Getters
    DirectX::XMFLOAT4 GetColorTint() const;
    std::shared_ptr<SimpleVertexShader> GetVertexShader();
    std::shared_ptr<SimplePixelShader> GetPixelShader();
    DirectX::XMFLOAT2 GetUVScale();
    DirectX::XMFLOAT2 GetUVOffset();

    // Setters
    void SetColorTint(DirectX::XMFLOAT4 newTint);
    void SetVertexShader(std::shared_ptr<SimpleVertexShader> newShader);
    void SetPixelShader(std::shared_ptr<SimplePixelShader> newShader);
    void SetUVScale(DirectX::XMFLOAT2 scale);
    void SetUVOffset(DirectX::XMFLOAT2 offset);

    void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);

private:
    DirectX::XMFLOAT4 colorTint;
    std::shared_ptr<SimpleVertexShader> vs;
    std::shared_ptr<SimplePixelShader> ps;

    DirectX::XMFLOAT2 uvOffset;
    DirectX::XMFLOAT2 uvScale;
};

