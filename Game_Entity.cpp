#include "Game_Entity.h"
//#include "BufferStructs.h"
#include "Graphics.h"

Game_Entity::Game_Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
    : 
    mesh(mesh),
    material(material)
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

void Game_Entity::Draw(std::shared_ptr<Camera> camera)
{
    //Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());
    material.get()->PrepareMaterial(transform, camera);
    mesh->Draw(Graphics::Context.Get());
}

std::shared_ptr<Material> Game_Entity::GetMaterial()
{
    return material;
}

void Game_Entity::setMaterial(std::shared_ptr<Material> _material)
{
    material = _material;
}


