#include "Material.h"
#include "SimpleShader.h"


Material::Material(
    DirectX::XMFLOAT4 colorTint,
    std::shared_ptr<SimpleVertexShader> vs,
    std::shared_ptr<SimplePixelShader> ps,
	DirectX::XMFLOAT2 uvOffset,
	DirectX::XMFLOAT2 uvScale)

    : 
	colorTint(colorTint),
    vs(vs),
    ps(ps),
	uvScale(uvScale),
	uvOffset(uvOffset)

{}

DirectX::XMFLOAT4 Material::GetColorTint() const { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return ps; }
DirectX::XMFLOAT2 Material::GetUVScale() { return DirectX::XMFLOAT2(); }
DirectX::XMFLOAT2 Material::GetUVOffset() { return DirectX::XMFLOAT2(); }

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
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->CopyAllBufferData();

	ps->SetFloat4("colorTint", colorTint);
	ps->SetFloat2("uvScale", uvScale);
	ps->SetFloat2("uvOffset", uvOffset);
	ps->CopyAllBufferData();

	// Loop and set any other resources
	//for (auto& t : textureSRVs) { ps->SetShaderResourceView(t.first.c_str(), t.second.Get()); }
	//for (auto& s : samplers) { ps->SetSamplerState(s.first.c_str(), s.second.Get()); }
}