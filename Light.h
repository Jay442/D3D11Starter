#pragma once
#include <DirectXMath.h>

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light
{
	int					Type;
	DirectX::XMFLOAT3	Direction;	// 16 bytes

	float				Range;
	DirectX::XMFLOAT3	Position;	// 32 bytes

	float				Intensity;
	DirectX::XMFLOAT3	Color;		// 48 bytes

	float				SpotInnerAngle;
	float				SpotOuterAngle;
	DirectX::XMFLOAT2	Padding;	// 64 bytes
};