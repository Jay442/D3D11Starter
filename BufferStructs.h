#pragma once
#include <d3d11.h> 
#include <DirectXMath.h>

struct VertexShaderData
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4 colorTint;
};