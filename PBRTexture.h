#pragma once
#include "WICTextureLoader.h" 

struct PBRTexture {
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Albedo = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Normal = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Roughness = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Metallic = nullptr;
};
