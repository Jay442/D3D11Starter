#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include "BufferStructs.h"
#include "Game_Entity.h"
#include "Camera.h"
#include <memory>
#include "WICTextureLoader.h" 
#include "Light.h"
#include "Sky.h"
#include "PBRTexture.h"

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
	void GenerateLights();
	void GenerateShadows();
	void RenderShadowMap();
	void PostProcessingReSize();
	void CreateRenderTarget(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv, Microsoft::WRL::ComPtr <ID3D11ShaderResourceView>& srv);
	void ImGuiUpdate(float deltaTime);
	void BuildUI();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(const std::wstring& path);
	PBRTexture LoadPBRMaterial(const std::wstring& basePath, const std::wstring& materialName);

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

	std::shared_ptr<Sky> sky;

	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Game_Entity>> entities;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<Light> lights;
	DirectX::XMFLOAT3 ambientColor;

	const UINT shadowMapResolution = 2048;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	std::shared_ptr<DirectX::XMFLOAT4X4> shadowView;
	std::shared_ptr<DirectX::XMFLOAT4X4> shadowProjection;
	std::shared_ptr<SimpleVertexShader> shadowVS;

	//Post Processing Fields
	std::shared_ptr<SimplePixelShader> blurPS;
	std::shared_ptr<SimpleVertexShader> fullscreenVS;

	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> ppVS;

	// Resources that are tied to a particular post process
	std::shared_ptr<SimplePixelShader> ppPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blurRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurSRV; // For sampling
	bool blurOn;
	int blurRadius;

	std::shared_ptr<SimplePixelShader> pixelizePS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pixelizeRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pixelizeSRV; // For sampling
	bool pixelizeOn = true;
	float pixelSize = 0.02f;

	std::shared_ptr<SimplePixelShader> copyPS;
};

