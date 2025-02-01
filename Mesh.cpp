#include "Mesh.h"

Mesh::~Mesh()
{
}

void Mesh::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
	vertexCount = static_cast<unsigned int>(vertices.size());
	indexCount = static_cast<unsigned int>(indices.size());

	{
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * vertexCount;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices.data();

		device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());
	}

	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(unsigned int) * indexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices.data();

		device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	}
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer()
{
    return vertexBuffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer()
{
    return indexBuffer;
}

unsigned int Mesh::GetIndexCount()
{
    return indexCount;
}

unsigned int Mesh::GetVertexCount()
{
    return vertexCount;
}

// DRAW geometry
// - These steps are generally repeated for EACH object you draw
// - Other Direct3D calls will also be necessary to do more complex things
void Mesh::Draw(ID3D11DeviceContext* context)
{
	// Set buffers in the input assembler (IA) stage
	//  - Do this ONCE PER OBJECT, since each object may have different geometry
	//  - For this demo, this step *could* simply be done once during Init()
	//  - However, this needs to be done between EACH DrawIndexed() call
	//     when drawing different geometry, so it's here as an example
    UINT stride = sizeof(Vertex); 
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Tell Direct3D to draw
	//  - Begins the rendering pipeline on the GPU
	//  - Do this ONCE PER OBJECT you intend to draw
	//  - This will use all currently set Direct3D resources (shaders, buffers, etc)
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER
	context->DrawIndexed(
		indexCount,     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}
