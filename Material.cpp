#include "Material.h"
#include "SimpleShader.h"


Material::Material(
    DirectX::XMFLOAT4 colorTint,
    std::shared_ptr<SimpleVertexShader> vs,
    std::shared_ptr<SimplePixelShader> ps,
	DirectX::XMFLOAT2 uvScale,
	DirectX::XMFLOAT2 uvOffset)

    : 
	colorTint(colorTint),
    vs(vs),
    ps(ps),
	uvOffset(uvOffset),
	uvScale(uvScale)

{}

DirectX::XMFLOAT4 Material::GetColorTint() const { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return ps; }
DirectX::XMFLOAT2 Material::GetUVScale() { return uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() { return uvOffset; }
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Material::GetTextureSRV(std::string name)
{
	// Search for the key
	auto it = textureSRVs.find(name);

	// Not found, return null
	if (it == textureSRVs.end())
		return 0;

	// Return the texture ComPtr
	return it->second;
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> Material::GetSampler(std::string name)
{
	// Search for the key
	auto it = samplers.find(name);

	// Not found, return null
	if (it == samplers.end())
		return 0;

	// Return the sampler ComPtr
	return it->second;
}
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& Material::GetTextureSRVMap() { return textureSRVs; }

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>>& Material::GetSamplerMap(){ return samplers; }

void Material::SetColorTint(DirectX::XMFLOAT4 newTint) { colorTint = newTint; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> newShader) { vs = newShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> newShader) { ps = newShader; }
void Material::SetUVScale(DirectX::XMFLOAT2 scale) { uvScale = scale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 offset) { uvOffset = offset; }

void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
	vs->SetShader();
	ps->SetShader();

	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTrans", transform->GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->CopyAllBufferData();

	ps->SetFloat4("colorTint", colorTint);
	ps->SetFloat2("uvScale", uvScale);
	ps->SetFloat2("uvOffset", uvOffset);
	ps->SetFloat3("cameraPosition", camera->GetTransform().GetPosition());
	ps->CopyAllBufferData();

	for (auto& t : textureSRVs) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { ps->SetSamplerState(s.first.c_str(), s.second); }
}

void Material::AddTextureSRV(const std::string& shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv){ textureSRVs.insert({ shaderVariableName, srv }); }
void Material::AddSampler(const std::string& shaderVariableName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler) { samplers.insert({ shaderVariableName, sampler }); }