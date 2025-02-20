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
	LoadShaders();
	CreateGeometry();

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
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	unsigned int size = sizeof(VertexShaderData);
	size = (size + 15) / 16 * 16;

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	Graphics::Device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());
	Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());


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


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
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

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	Vertex squareVertices[] = {
		{ XMFLOAT3(-0.5f, +0.0f, +0.0f), yellow },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), yellow },
		{ XMFLOAT3(-1.0f, -0.5f, +0.0f), purple },
		{ XMFLOAT3(-1.0f, +0.0f, +0.0f), purple }
	};

	Vertex pentagonVertices[] = {
		{ XMFLOAT3(+0.5f, +1.0f, +0.0f), green },
		{ XMFLOAT3(+0.8f, +0.7f, +0.0f), blue },
		{ XMFLOAT3(+0.8f, +0.1f, +0.0f), red },
		{ XMFLOAT3(+0.2f, +0.1f, +0.0f), purple },
		{ XMFLOAT3(+0.2f, +0.7f, +0.0f), yellow }
	};
	std::vector<Vertex> vertexVector(std::begin(vertices), std::end(vertices));
	std::vector<Vertex> squareVertexVector(std::begin(squareVertices), std::end(squareVertices));
	std::vector<Vertex> pentagonVertexVector(std::begin(pentagonVertices), std::end(pentagonVertices));

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };
	unsigned int squareIndices[] = { 0, 1, 2, 0, 2, 3 };
	unsigned int pentagonIndices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4 };
	std::vector<unsigned int> indicesVector(std::begin(indices), std::end(indices));
	std::vector<unsigned int> squareIndiciesVector(std::begin(squareIndices), std::end(squareIndices));
	std::vector<unsigned int> pentagonIndiciesVector(std::begin(pentagonIndices), std::end(pentagonIndices));

	triangle = std::make_shared<Mesh>();
	triangle->Initialize(Graphics::Device, vertexVector, indicesVector);
	meshes.push_back(triangle);

	square = std::make_shared<Mesh>();
	square->Initialize(Graphics::Device, squareVertexVector, squareIndiciesVector);
	meshes.push_back(square);

	pentagon = std::make_shared<Mesh>();
	pentagon->Initialize(Graphics::Device, pentagonVertexVector, pentagonIndiciesVector);
	meshes.push_back(pentagon);

	std::shared_ptr<Game_Entity> entity1 = std::make_shared<Game_Entity>(triangle);
	std::shared_ptr<Game_Entity> entity2 = std::make_shared<Game_Entity>(square);
	std::shared_ptr<Game_Entity> entity3 = std::make_shared<Game_Entity>(pentagon);	  
	std::shared_ptr<Game_Entity> entity4 = std::make_shared<Game_Entity>(pentagon);	  
	std::shared_ptr<Game_Entity> entity5 = std::make_shared<Game_Entity>(pentagon);

	entities.push_back(entity1);
	entities.push_back(entity2);
	entities.push_back(entity3);
	entities.push_back(entity4);
	entities.push_back(entity5);
														   

	// Create a VERTEX BUFFER
	// - This holds the vertex data of triangles for a single object
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
	{
		// First, we need to describe the buffer we want Direct3D to make on the GPU
		//  - Note that this variable is created on the stack since we only need it once
		//  - After the buffer is created, this description variable is unnecessary
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		vbd.ByteWidth = sizeof(Vertex) * 3;       // 3 = number of vertices in the buffer
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
		vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		// Create the proper struct to hold the initial vertex data
		// - This is how we initially fill the buffer with data
		// - Essentially, we're specifying a pointer to the data to copy
		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices; // pSysMem = Pointer to System Memory

		// Actually create the buffer on the GPU with the initial data
		// - Once we do this, we'll NEVER CHANGE DATA IN THE BUFFER AGAIN
		Graphics::Device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());
	}

	// Create an INDEX BUFFER
	// - This holds indices to elements in the vertex buffer
	// - This is most useful when vertices are shared among neighboring triangles
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
	{
		// Describe the buffer, as we did above, with two major differences
		//  - Byte Width (3 unsigned integers vs. 3 whole vertices)
		//  - Bind Flag (used as an index buffer instead of a vertex buffer) 
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		ibd.ByteWidth = sizeof(unsigned int) * 3;	// 3 = number of indices in the buffer
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
		ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		// Specify the initial data for this buffer, similar to above
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices; // pSysMem = Pointer to System Memory

		// Actually create the buffer with the initial data
		// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
		Graphics::Device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	}
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	ImGuiUpdate(deltaTime);
	BuildUI();

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
		// Triangle Info
		if (ImGui::CollapsingHeader("Triangle Info"))
		{
			ImGui::Text("Triangles: %d", (int)triangle->GetIndexCount()/3);
			ImGui::Text("Vertices: %d", (int)triangle->GetVertexCount());
			ImGui::Text("Indices: %d", (int)triangle->GetIndexCount());
		}

		// Square Info
		if (ImGui::CollapsingHeader("Square Info"))
		{
			ImGui::Text("Triangles: %d", (int)triangle->GetIndexCount() / 3);
			ImGui::Text("Vertices: %d", (int)square->GetVertexCount());
			ImGui::Text("Indices: %d", (int)square->GetIndexCount());
		}

		// Pentagon Info
		if (ImGui::CollapsingHeader("Pentagon Info"))
		{
			ImGui::Text("Triangles: %d", (int)triangle->GetIndexCount() / 3);
			ImGui::Text("Vertices: %d", (int)pentagon->GetVertexCount());
			ImGui::Text("Indices: %d", (int)pentagon->GetIndexCount());
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

	if (ImGui::CollapsingHeader("Entity Transform", ImGuiTreeNodeFlags_DefaultOpen))
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
		e->Draw(vsConstantBuffer);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		ImGui::Render(); // Turns this frame�s UI into renderable triangles
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



