#pragma once
#include "Transform.h"
#include "Mesh.h"
#include <memory>

class Game_Entity
{
public:
	Game_Entity(std::shared_ptr<Mesh> mesh);
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();

	void SetMesh(std::shared_ptr<Mesh> mesh);
	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer);

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};

