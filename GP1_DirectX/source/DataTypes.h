#pragma once
#include "Math.h"

namespace dae
{
	struct Vertex_In
	{
		Vector3 pos;
		ColorRGB col{ colors::White };
		Vector2 uv;
		Vector3 norm;
		Vector3 tan;
	};
}
