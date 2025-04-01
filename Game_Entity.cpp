#include "Game_Entity.h"
#include "BufferStructs.h"
#include "Graphics.h"

Game_Entity::Game_Entity(std::shared_ptr<Mesh> mesh)
    : mesh(mesh)
{
    transform = std::make_shared<Transform>();
}

std::shared_ptr<Transform> Game_Entity::GetTransform() {
    return transform;
}

std::shared_ptr<Mesh> Game_Entity::GetMesh() {
    return mesh;
}

void Game_Entity::SetMesh(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
}

void Game_Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer, std::shared_ptr<Camera> camera)
{
    VertexShaderData vsData = {};
    vsData.colorTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    vsData.world = transform->GetWorldMatrix();

    vsData.view = camera->GetViewMatrix();
    vsData.projection = camera->GetProjectionMatrix();

    D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
    Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
    Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);

    Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());

    mesh->Draw(Graphics::Context.Get());
}
