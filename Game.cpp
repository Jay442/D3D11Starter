#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include <DirectXMath.h>
#include "SimpleShader.h"
#include "Material.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	//LoadShaders();
	CreateGeometry();
	GenerateLights();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		//Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		//Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		//Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	//unsigned int size = sizeof(VertexShaderData);
	//size = (size + 15) / 16 * 16;
	//
	//D3D11_BUFFER_DESC cbDesc = {};
	//cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cbDesc.ByteWidth = size;
	//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	//cbDesc.MiscFlags = 0;
	//cbDesc.StructureByteStride = 0;

	//Graphics::Device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());
	//Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());

	cameras.push_back(std::make_shared<Camera>(
		Window::AspectRatio(),
		XMFLOAT3(0, 5, -30),     // Position
		XMFLOAT3(0, 0, 0),      // Rotation
		XM_PIDIV4,              // 45° FOV
		0.01f,                  // Near clip
		100.0f,                 // Far clip
		5.0f,                   // Move speed
		0.002f                  // Mouse sensitivity
	));

	cameras.push_back(std::make_shared<Camera>(
		Window::AspectRatio(),
		XMFLOAT3(0, 2, -10),    
		XMFLOAT3(0.2f, 0, 0),    
		XM_PIDIV4 * 1.5,               
		0.01f,
		100.0f,
		8.0f,                    
		0.001f                   
	));

	// Add a third camera if you want
	cameras.push_back(std::make_shared<Camera>(
		Window::AspectRatio(),
		XMFLOAT3(-5, 1, 0),      
		XMFLOAT3(0, XM_PIDIV2, 0), 
		XM_PIDIV4 * 1.2f,        
		0.01f,
		100.0f,
		3.0f,                    
		0.003f                   
	));
	activeCameraIndex = 0;
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Game::LoadTexture(const std::wstring& path)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(path).c_str(), 0, &srv);
	return srv;
}

PBRTexture Game::LoadPBRMaterial(
	const std::wstring& basePath,
	const std::wstring& materialName
) {
	PBRTexture tex;

	tex.Albedo = LoadTexture(basePath + materialName + L"_Color.png");
	tex.Normal = LoadTexture(basePath + materialName + L"_NormalDX.png");
	tex.Roughness = LoadTexture(basePath + materialName + L"_Roughness.png");
	tex.Metallic = LoadTexture(basePath + materialName + L"_Metalness.png");

	return tex;
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 purple = XMFLOAT4(0.5f, 0.0f, 0.5f, 1.0f);
	XMFLOAT4 none = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	std::shared_ptr<SimpleVertexShader> vs = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> ps = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> normalps = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> uvps = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> custom = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> combined = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CombinationPS.cso").c_str());
	std::shared_ptr<SimpleVertexShader> skyVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyVS.cso").c_str());
	std::shared_ptr<SimplePixelShader> skyPS = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"SkyPS.cso").c_str());


	// Sampler Loading 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16; 

	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;

	Graphics::Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	// Mesh Loading
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>("Cube", FixPath("../../Assets/Models/cube.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/Models/cylinder.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>("Helix", FixPath("../../Assets/Models/helix.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>("Sphere", FixPath("../../Assets/Models/sphere.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>("Torus", FixPath("../../Assets/Models/torus.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>("Quad", FixPath("../../Assets/Models/quad.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> quad2sidedMesh = std::make_shared<Mesh>("Double-Sided Quad", FixPath("../../Assets/Models/quad_double_sided.obj").c_str(), Graphics::Device);

	std::shared_ptr<Mesh> cubeMeshNormal = std::make_shared<Mesh>("Cube", FixPath("../../Assets/Models/cube.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> cylinderMeshNormal = std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/Models/cylinder.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> helixMeshNormal = std::make_shared<Mesh>("Helix", FixPath("../../Assets/Models/helix.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> sphereMeshNormal = std::make_shared<Mesh>("Sphere", FixPath("../../Assets/Models/sphere.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> torusMeshNormal = std::make_shared<Mesh>("Torus", FixPath("../../Assets/Models/torus.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> quadMeshNormal = std::make_shared<Mesh>("Quad", FixPath("../../Assets/Models/quad.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> quad2sidedMeshNormal = std::make_shared<Mesh>("Double-Sided Quad", FixPath("../../Assets/Models/quad_double_sided.obj").c_str(), Graphics::Device);

	std::shared_ptr<Mesh> cubeMeshUV = std::make_shared<Mesh>("Cube", FixPath("../../Assets/Models/cube.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> cylinderMeshUV = std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/Models/cylinder.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> helixMeshUV = std::make_shared<Mesh>("Helix", FixPath("../../Assets/Models/helix.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> sphereMeshUV = std::make_shared<Mesh>("Sphere", FixPath("../../Assets/Models/sphere.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> torusMeshUV = std::make_shared<Mesh>("Torus", FixPath("../../Assets/Models/torus.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> quadMeshUV = std::make_shared<Mesh>("Quad", FixPath("../../Assets/Models/quad.obj").c_str(), Graphics::Device);
	std::shared_ptr<Mesh> quad2sidedMeshUV = std::make_shared<Mesh>("Double-Sided Quad", FixPath("../../Assets/Models/quad_double_sided.obj").c_str(), Graphics::Device);

	meshes.insert(meshes.end(), {
		cubeMesh, cylinderMesh, helixMesh, sphereMesh, torusMesh, quadMesh, quad2sidedMesh,
		cubeMeshNormal, cylinderMeshNormal, helixMeshNormal, sphereMeshNormal, torusMeshNormal, quadMeshNormal, quad2sidedMeshNormal,
		cubeMeshUV, cylinderMeshUV, helixMeshUV, sphereMeshUV, torusMeshUV, quadMeshUV, quad2sidedMeshUV
		});

	// Texture Loading
	const std::wstring& basePath = L"../../Assets/Textures/";
	PBRTexture bambooTexture = LoadPBRMaterial(basePath, L"Bamboo001A_1K-PNG");
	PBRTexture chipTexture = LoadPBRMaterial(basePath, L"Chip005_1K-PNG");
	PBRTexture metalPlateTexture = LoadPBRMaterial(basePath, L"MetalPlates006_1K-PNG");
	PBRTexture rockTexture = LoadPBRMaterial(basePath, L"Rock051_1K-PNG");

	// Sky
	sky = std::make_shared<Sky>(
		FixPath(L"../../Assets/Skies/Clouds Pink/right.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Pink/left.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Pink/up.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Pink/down.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Pink/front.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds Pink/back.png").c_str(),
		cubeMesh,
		skyVS,
		skyPS,
		samplerState);

	// Material Loading
	std::shared_ptr<Material> normalMat = std::make_shared<Material>(none, vs, normalps);
	std::shared_ptr<Material> uvMat = std::make_shared<Material>(none, vs, uvps);
	std::shared_ptr<Material> customMat = std::make_shared<Material>(none, vs, custom);

	std::shared_ptr<Material> bamboo = std::make_shared<Material>(none, vs, ps);
	bamboo->AddSampler("BasicSampler", samplerState);
	bamboo->AddTextureSRV("Albedo", bambooTexture.Albedo);
	bamboo->AddTextureSRV("NormalMap", bambooTexture.Normal);
	bamboo->AddTextureSRV("RoughnessMap", bambooTexture.Roughness);
	bamboo->AddTextureSRV("MetalnessMap", bambooTexture.Metallic);

	std::shared_ptr<Material> chip = std::make_shared<Material>(none, vs, ps);
	chip->AddSampler("BasicSampler", samplerState);
	chip->AddTextureSRV("Albedo", chipTexture.Albedo);
	chip->AddTextureSRV("NormalMap", chipTexture.Normal);
	chip->AddTextureSRV("RoughnessMap", chipTexture.Roughness);
	chip->AddTextureSRV("MetalnessMap", chipTexture.Metallic);

	std::shared_ptr<Material> metalPlate = std::make_shared<Material>(none, vs, ps);
	metalPlate->AddSampler("BasicSampler", samplerState);
	metalPlate->AddTextureSRV("Albedo", metalPlateTexture.Albedo);
	metalPlate->AddTextureSRV("NormalMap", metalPlateTexture.Normal);
	metalPlate->AddTextureSRV("RoughnessMap", metalPlateTexture.Roughness);
	metalPlate->AddTextureSRV("MetalnessMap", metalPlateTexture.Metallic);

	std::shared_ptr<Material> rock = std::make_shared<Material>(none, vs, ps);
	rock->AddSampler("BasicSampler", samplerState);
	rock->AddTextureSRV("Albedo", rockTexture.Albedo);
	rock->AddTextureSRV("NormalMap", rockTexture.Normal);
	rock->AddTextureSRV("RoughnessMap", rockTexture.Roughness);
	rock->AddTextureSRV("MetalnessMap", rockTexture.Metallic);

	

	materials.insert(materials.end(), { bamboo, chip, metalPlate, rock, normalMat, uvMat, customMat });

	//mat1->AddTextureSRV("SurfaceTexture", woodTexture);
	//mat1->AddTextureSRV("NormalMap", woodTextureNormal);
	//mat1->AddSampler("BasicSampler", samplerState);
	//mat2->AddTextureSRV("SurfaceTexture", rockyTexture);
	//mat2->AddTextureSRV("NormalMap", rockyTextureNormal);
	//mat2->AddSampler("BasicSampler", samplerState);
	//mat3->AddTextureSRV("TextureA", woodTexture);
	//mat3->AddTextureSRV("TextureB", stainDecal);
	//mat3->AddSampler("BasicSampler", samplerState);
				

	entities.push_back(std::make_shared<Game_Entity>(cubeMesh, chip));
	entities.push_back(std::make_shared<Game_Entity>(cylinderMesh, metalPlate));
	entities.push_back(std::make_shared<Game_Entity>(helixMesh, chip));
	entities.push_back(std::make_shared<Game_Entity>(sphereMesh, bamboo));
	entities.push_back(std::make_shared<Game_Entity>(torusMesh, metalPlate));
	entities.push_back(std::make_shared<Game_Entity>(quadMesh, chip));
	entities.push_back(std::make_shared<Game_Entity>(quad2sidedMesh, bamboo));

	entities.push_back(std::make_shared<Game_Entity>(cubeMeshNormal, normalMat));
	entities.push_back(std::make_shared<Game_Entity>(cylinderMeshNormal, normalMat));
	entities.push_back(std::make_shared<Game_Entity>(helixMeshNormal, normalMat));
	entities.push_back(std::make_shared<Game_Entity>(sphereMeshNormal, normalMat));
	entities.push_back(std::make_shared<Game_Entity>(torusMeshNormal, normalMat));
	entities.push_back(std::make_shared<Game_Entity>(quadMeshNormal, normalMat));
	entities.push_back(std::make_shared<Game_Entity>(quad2sidedMeshNormal, normalMat));

	entities.push_back(std::make_shared<Game_Entity>(cubeMeshUV, uvMat));
	entities.push_back(std::make_shared<Game_Entity>(cylinderMeshUV, uvMat));
	entities.push_back(std::make_shared<Game_Entity>(helixMeshUV, uvMat));
	entities.push_back(std::make_shared<Game_Entity>(sphereMeshUV, uvMat));
	entities.push_back(std::make_shared<Game_Entity>(torusMeshUV, uvMat));
	entities.push_back(std::make_shared<Game_Entity>(quadMeshUV, uvMat));
	entities.push_back(std::make_shared<Game_Entity>(quad2sidedMeshUV, uvMat));

	entities[0]->GetTransform()->MoveAbsolute(-9, 0, 0);
	entities[1]->GetTransform()->MoveAbsolute(-6, 0, 0);
	entities[2]->GetTransform()->MoveAbsolute(-3, 0, 0);
	entities[3]->GetTransform()->MoveAbsolute(0, 0, 0);
	entities[4]->GetTransform()->MoveAbsolute(3, 0, 0);
	entities[5]->GetTransform()->MoveAbsolute(6, 0, 0);
	entities[6]->GetTransform()->MoveAbsolute(9, 0, 0);
	entities[7]->GetTransform()->MoveAbsolute(-9, 10, 0);
	entities[8]->GetTransform()->MoveAbsolute(-6, 10, 0);
	entities[9]->GetTransform()->MoveAbsolute(-3, 10, 0);
	entities[10]->GetTransform()->MoveAbsolute(0, 10, 0);
	entities[11]->GetTransform()->MoveAbsolute(3, 10, 0);
	entities[12]->GetTransform()->MoveAbsolute(6, 10, 0);
	entities[13]->GetTransform()->MoveAbsolute(9, 10, 0);
	entities[14]->GetTransform()->MoveAbsolute(-9, 5, 0);
	entities[15]->GetTransform()->MoveAbsolute(-6, 5, 0);
	entities[16]->GetTransform()->MoveAbsolute(-3, 5, 0);
	entities[17]->GetTransform()->MoveAbsolute(0, 5, 0);
	entities[18]->GetTransform()->MoveAbsolute(3, 5, 0);
	entities[19]->GetTransform()->MoveAbsolute(6, 5, 0);
	entities[20]->GetTransform()->MoveAbsolute(9, 5, 0);

	ambientColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void Game::GenerateLights()
{
	lights.clear();

	// Directional light
	Light dir1 = {};
	dir1.Type = LIGHT_TYPE_DIRECTIONAL;
	dir1.Direction = XMFLOAT3(1, -1, 0);
	dir1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	dir1.Intensity = 1.0f;
	lights.push_back(dir1);
	
	Light dir2 = {};
	dir2.Type = LIGHT_TYPE_DIRECTIONAL;
	dir2.Direction = XMFLOAT3(1, 1, 0);
	dir2.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	dir2.Intensity = 1.0f;
	lights.push_back(dir2);
	
	Light dir3 = {};
	dir3.Type = LIGHT_TYPE_DIRECTIONAL;
	dir3.Direction = XMFLOAT3(0, 0, 1);
	dir3.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	dir3.Intensity = 1.0f;
	lights.push_back(dir3);
	
	// Point light
	Light point1 = {};
	point1.Type = LIGHT_TYPE_POINT;
	point1.Position = XMFLOAT3(0, 2, 0);
	point1.Color = XMFLOAT3(1, 0, 0);
	point1.Intensity = 2.0f;
	point1.Range = 10.0f;
	lights.push_back(point1);
	
	// Spot light 
	Light spot1 = {};
	spot1.Type = LIGHT_TYPE_SPOT;
	spot1.Position = XMFLOAT3(6, 2, 0);
	spot1.Direction = XMFLOAT3(0, -1, 0);
	spot1.Color = XMFLOAT3(0, 0, 1);
	spot1.Intensity = 5.0f;
	spot1.Range = 5.0f;
	spot1.SpotFalloff = 1.0f;
	lights.push_back(spot1);

}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	float aspectRatio = Window::AspectRatio();
	for (std::shared_ptr<Camera>& camera : cameras)
	{
		if (camera) camera->UpdateProjectionMatrix(aspectRatio);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	ImGuiUpdate(deltaTime);
	BuildUI();

	activeCamera = GetActiveCamera();
	if (activeCamera)
	{
		activeCamera->Update(deltaTime);
	}

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}

void Game::ImGuiUpdate(float deltaTime)
{
	// Put this all in a helper method that is called from Game::Update()
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	//ImGui::ShowDemoWindow();
}

void Game::BuildUI()
{
	ImGui::Begin("My First Window"); // Everything after is part of the window
	ImGui::Text("This text is in the window");
	ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
	ImGui::Text("Window Size: %dx%d", Window::Width(), Window::Height());

	ImGui::ColorEdit4("Background Color", color);

	
	// Mesh Info
	if (ImGui::CollapsingHeader("Meshes"))
	{
		
	}

	// Cameras
	if (ImGui::CollapsingHeader("Switch Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (int i = 0; i < cameras.size(); i++)
		{
			std::string label = "Camera " + std::to_string(i + 1);
			if (ImGui::RadioButton(label.c_str(), activeCameraIndex == i))
			{
				activeCameraIndex = i;
			}

			// Show camera position as a tooltip
			if (ImGui::IsItemHovered())
			{
				XMFLOAT3 pos = cameras[i]->GetTransform().GetPosition();
				ImGui::SetTooltip("Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
			}
		}
	}


	if (ImGui::Button("Show ImGui Window"))
	{
		demoWindowVisible = !demoWindowVisible;
	}

	if (demoWindowVisible)
	{
		ImGui::ShowDemoWindow();
	}

	//if (ImGui::CollapsingHeader("Shader Controls"))
	//{
	//	ImGui::SliderFloat3("Offset", (float*)&vsData.offset, -1.0f, 1.0f);
	//
	//	ImGui::ColorEdit4("Color Tint", (float*)&vsData.colorTint);
	//}

	if (ImGui::CollapsingHeader("Entity Transform"))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			if (ImGui::CollapsingHeader(("Entity " + std::to_string(i + 1)).c_str()))
			{
				// Position
				DirectX::XMFLOAT3 position = entities[i]->GetTransform()->GetPosition();
				if (ImGui::DragFloat3(("Position##" + std::to_string(i)).c_str(), &position.x, 0.1f))
				{
					entities[i]->GetTransform()->SetPosition(position);
				}

				// Rotation
				DirectX::XMFLOAT3 rotation = entities[i]->GetTransform()->GetPitchYawRoll();
				if (ImGui::DragFloat3(("Rotation##" + std::to_string(i)).c_str(), &rotation.x, 1.0f))
				{
					entities[i]->GetTransform()->SetRotation(rotation);
				}

				// Scale
				DirectX::XMFLOAT3 scale = entities[i]->GetTransform()->GetScale();
				if (ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &scale.x, 0.1f))
				{
					entities[i]->GetTransform()->SetScale(scale);
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Materials"))
	{
		for (size_t i = 0; i < materials.size(); ++i)
		{
			auto& mat = materials[i];

			if (ImGui::CollapsingHeader(("Material " + std::to_string(i)).c_str()))
			{
				// Color Tint Control
				XMFLOAT4 tint = mat->GetColorTint();
				if (ImGui::ColorEdit4("Color Tint", &tint.x))
					mat->SetColorTint(tint);
		
				// UV Controls
				XMFLOAT2 scale = mat->GetUVScale();
				if (ImGui::DragFloat2("UV Scale", &scale.x, 0.01f, 0.0f, 5.0f))
					mat->SetUVScale(scale);

				XMFLOAT2 offset = mat->GetUVOffset();
				if (ImGui::DragFloat2("UV Offset", &offset.x, 0.01f, -1.0f, 1.0f))
					mat->SetUVOffset(offset);

				// Textures
				ImGui::Separator();
				ImGui::Text("Textures:");

				for (auto& it : mat->GetTextureSRVMap())
				{
					ImGui::Text(it.first.c_str());
					if (it.second)
					{
						ImGui::Image((ImTextureID)it.second.Get(), ImVec2(256, 256));
					}
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Lights"))
	{
		ImGui::ColorEdit3("Ambient Color", &ambientColor.x);

		for (int i = 0; i < lights.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::CollapsingHeader(("Light " + std::to_string(i)).c_str()))
			{
				ImGui::Combo("Type", &lights[i].Type, "Directional\0Point\0Spot\0");

				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.1f, 0.0f, 10.0f);

				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL)
				{
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.1f);
					ImGui::DragFloat("Range", &lights[i].Range, 0.1f, 0.0f, 100.0f);
				}

				if (lights[i].Type != LIGHT_TYPE_POINT)
				{
					ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.1f);
				}

				if (lights[i].Type == LIGHT_TYPE_SPOT)
				{
					ImGui::DragFloat("Fall Off", &lights[i].SpotFalloff, 0.01f, 0.0f, XM_PI);
				}
			}
			ImGui::PopID();
		}
	}


	ImGui::End();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	
	
	for (auto& e : entities)
	{
		std::shared_ptr<SimplePixelShader> ps = e->GetMaterial()->GetPixelShader();
		ps->SetFloat3("ambientColor", ambientColor);
		ps->SetInt("lightCount", (int)lights.size());
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		e->Draw(activeCamera);
	}

	sky->Draw(activeCamera);
	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

std::shared_ptr<Camera> Game::GetActiveCamera() const
{
	if (cameras.empty()) return nullptr;
	return cameras[activeCameraIndex];
}

