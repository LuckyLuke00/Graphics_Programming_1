#pragma once

#include "Math.h"

namespace dae
{
	struct Vertex_In
	{
		Vector3 pos{};
		Vector3 norm{};
		Vector3 tan{};
		Vector2 uv{};
		ColorRGB col{ colors::White };
	};

	struct Vertex_Out
	{
		Vector4 pos{};
		Vector3 norm{};
		Vector3 tan{};
		Vector2 uv{};
		ColorRGB col{ colors::White };
		Vector3 view{};
	};

	struct LightingData
	{
		ColorRGB ambient{ .025f, .025f, .025f };
		float intensity{ 7.f };
		float shininess{ 25.f };
		Vector3 direction{ .577f, -.577f, .577f };
	};

	enum class CullMode
	{
		Back,
		Front,
		None,
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};
};
