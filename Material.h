#pragma once
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>
#include "Transform.h"
#include "Camera.h"
#include <unordered_map>

class Material
{
public:
    // Constructor
    Material(
        DirectX::XMFLOAT4 colorTint,
        std::shared_ptr<SimpleVertexShader> vs,
        std::shared_ptr<SimplePixelShader> ps,
        DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1.0f, 1.0f),
        DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0.0f, 0.0f));

    // Getters
    DirectX::XMFLOAT4 GetColorTint() const;
    std::shared_ptr<SimpleVertexShader> GetVertexShader();
    std::shared_ptr<SimplePixelShader> GetPixelShader();
    DirectX::XMFLOAT2 GetUVScale();
    DirectX::XMFLOAT2 GetUVOffset();
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureSRV(std::string name);
    Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSampler(std::string name);
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTextureSRVMap();
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>>& GetSamplerMap();

    // Setters
    void SetColorTint(DirectX::XMFLOAT4 newTint);
    void SetVertexShader(std::shared_ptr<SimpleVertexShader> newShader);
    void SetPixelShader(std::shared_ptr<SimplePixelShader> newShader);
    void SetUVScale(DirectX::XMFLOAT2 scale);
    void SetUVOffset(DirectX::XMFLOAT2 offset);

    void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);
    void AddTextureSRV(const std::string& shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
    void AddSampler(const std::string& shaderVariableName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

private:
    DirectX::XMFLOAT4 colorTint;
    std::shared_ptr<SimpleVertexShader> vs;
    std::shared_ptr<SimplePixelShader> ps;

    DirectX::XMFLOAT2 uvOffset;
    DirectX::XMFLOAT2 uvScale;

    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

