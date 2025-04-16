#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include "BufferStructs.h"
#include "Game_Entity.h"
#include "Camera.h"
#include <memory>
#include "WICTextureLoader.h" 

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:
	float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	bool demoWindowVisible = false;
	float progress = 0.0f;

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	//void LoadShaders();
	void CreateGeometry();
	void ImGuiUpdate(float deltaTime);
	void BuildUI();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	//Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;

	std::vector<std::shared_ptr<Camera>> cameras;
	std::shared_ptr<Camera> activeCamera;
	int activeCameraIndex = 0;
	std::shared_ptr<Camera> GetActiveCamera() const;

	std::shared_ptr<Mesh> triangle;
	std::shared_ptr<Mesh> square;
	std::shared_ptr<Mesh> pentagon;

	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Game_Entity>> entities;
	std::vector<std::shared_ptr<Material>> materials;
};

