#pragma once
#include "Transform.h"
#include "Mesh.h"
#include <memory>
#include "Camera.h"
#include "Material.h"

class Game_Entity
{
public:
	Game_Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> material;

	std::shared_ptr<Material> getMaterial();

	void setMaterial(std::shared_ptr<Material> _material);
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void Draw(std::shared_ptr<Camera> camera);

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};

