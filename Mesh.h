#pragma once
#include "Vertex.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

class Mesh
{
public:
	Mesh() = default;
	~Mesh();
	Mesh(const Mesh&) = delete; // Remove copy constructor
	Mesh& operator=(const Mesh&) = delete; // Remove copy-assignment operator

	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	unsigned int GetVertexCount();
	void Draw(ID3D11DeviceContext* context);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	unsigned int indexCount = 0;
	unsigned int vertexCount = 0;
};

