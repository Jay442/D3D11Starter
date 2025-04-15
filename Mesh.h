#pragma once
#include "Vertex.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>

class Mesh
{
public:
	Mesh() = default;
	Mesh(const char* name, const std::string& objFile, Microsoft::WRL::ComPtr<ID3D11Device> device);
	~Mesh();
	Mesh(const Mesh&) = delete; // Remove copy constructor
	Mesh& operator=(const Mesh&) = delete; // Remove copy-assignment operator

	void Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, Vertex* vertArray, unsigned int vertexCount, unsigned int* indexArray, unsigned int indexCount);

	const char* GetName();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	unsigned int GetVertexCount();
	void Draw(ID3D11DeviceContext* context);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	unsigned int indexCount;
	unsigned int vertexCount;
	const char* name;
	void CreateBuffers(Microsoft::WRL::ComPtr<ID3D11Device> device, Vertex* vertArray, unsigned int vertexCount, unsigned int* indexArray, unsigned int indexCount);
};

