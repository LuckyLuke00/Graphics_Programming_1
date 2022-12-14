//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Math.h"
#include "Matrix.h"
#include "Renderer.h"
#include "Texture.h"
#include "Utils.h"

#include <algorithm>

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow{ pWindow },
	m_MeshRotationAngle{ 57.5f },
	m_pGlossMap{ Texture::LoadFromFile("Resources/vehicle_gloss.png") },
	m_pNormalMap{ Texture::LoadFromFile("Resources/vehicle_normal.png") },
	m_pSpecularMap{ Texture::LoadFromFile("Resources/vehicle_specular.png") },
	m_pTexture{ Texture::LoadFromFile("Resources/vehicle_diffuse.png") },
	m_LightDirection{ .577f, -.577f, .577f }
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_fWidth = static_cast<float>(m_Width);
	m_fHeight = static_cast<float>(m_Height);

	m_AspectRatio = m_fWidth / m_fHeight;

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = static_cast<uint32_t*>(m_pBackBuffer->pixels);
	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(m_AspectRatio, 45.f);

	const Vector3 position{ .0f, .0f, 50.f };
	constexpr float angle{ 90.f * TO_RADIANS };
	const Matrix worldMatrix{ Matrix::CreateRotationY(angle) * Matrix::CreateTranslation(position) };
	InitializeMesh("Resources/vehicle.obj", worldMatrix);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	m_pDepthBufferPixels = nullptr;

	if (m_pGlossMap)
	{
		delete m_pGlossMap;
		m_pGlossMap = nullptr;
	}

	if (m_pNormalMap)
	{
		delete m_pNormalMap;
		m_pNormalMap = nullptr;
	}

	if (m_pSpecularMap)
	{
		delete m_pSpecularMap;
		m_pSpecularMap = nullptr;
	}

	if (m_pTexture)
	{
		delete m_pTexture;
		m_pTexture = nullptr;
	}
}

void Renderer::Update(const Timer* pTimer)
{
	m_Camera.Update(pTimer);

	if (!m_RotateMesh) return;

	for (Mesh& mesh : m_Meshes)
	{
		mesh.RotateY(m_MeshRotationAngle * pTimer->GetElapsed());
	}
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	ClearBuffers(100, 100, 100);
	VertexTransformationFunction(m_Meshes);
	RenderMesh(m_Meshes[0]);

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::ClearBuffers(const Uint8& r, const Uint8& g, const Uint8& b) const
{
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, r, g, b));
}

void Renderer::ToggleDepthBuffer()
{
	m_RenderDepthBuffer = !m_RenderDepthBuffer;
}

void Renderer::ToggleRotation()
{
	m_RotateMesh = !m_RotateMesh;
}

void dae::Renderer::ToggleNormalMap()
{
	m_RenderNormalMap = !m_RenderNormalMap;
}

void dae::Renderer::CycleShadingMode()
{
	static constexpr int enumSize{ sizeof(ShadingMode) };
	m_CurrentShadingMode = static_cast<ShadingMode>((static_cast<int>(m_CurrentShadingMode) + 1) % enumSize);
}

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes) const
{
	// Precompute the viewProjectionMatrix for each mesh
	Matrix viewProjectionMatrix{ m_Camera.viewMatrix * m_Camera.projectionMatrix };

	// Compute half the width and height of the screen
	const float halfWidth{ m_fWidth * .5f };
	const float halfHeight{ m_fHeight * .5f };

	// Iterate over each mesh
	for (Mesh& mesh : meshes)
	{
		// Precompute the viewProjectionMatrix for this mesh.
		viewProjectionMatrix = mesh.worldMatrix * viewProjectionMatrix;

		// Use a reference to the mesh vertices and vertices_out vectors
		// to avoid repeated calls to the mesh accessor functions.
		const std::vector<Vertex>& vertices{ mesh.vertices };
		std::vector<Vertex_Out>& verticesOut{ mesh.vertices_out };

		// If the vertices_out vector is empty, initialize it to be the same size as the vertices vector
		// and copy the vertex color, UV, normal, and tangent data into the vertices_out vector.
		if (verticesOut.empty())
		{
			verticesOut.reserve(vertices.size());
			for (const Vertex& vertex : vertices)
			{
				verticesOut.emplace_back(Vertex_Out{ {}, vertex.color, vertex.uv, vertex.normal, vertex.tangent });
			}
		}

		// Iterate over the vertices of the mesh using a range-based for loop.
		for (int i{ 0 }; Vertex_Out & vertex : verticesOut)
		{
			// Transform the vertex position using the precomputed viewProjectionMatrix
			vertex.position = viewProjectionMatrix.TransformPoint({ vertices[i].position, 1.f });

			// Transform the normal and tangent vectors using the world matrix of the mesh
			vertex.normal = mesh.worldMatrix.TransformVector(vertices[i].normal);
			vertex.tangent = mesh.worldMatrix.TransformVector(vertices[i].tangent);

			// Compute the view direction vector as the difference between the transformed vertex position
			// and the origin of the camera.
			vertex.viewDirection = mesh.worldMatrix.TransformPoint(vertices[i].position) - m_Camera.origin;

			// Divide the x, y, and z coordinates of the position by the w coordinate.
			vertex.position.x /= vertex.position.w;
			vertex.position.y /= vertex.position.w;
			vertex.position.z /= vertex.position.w;

			// Transform the x and y coordinates of the position from normalized device coordinates
			// to screen space coordinates.
			vertex.position.x = (vertex.position.x + 1.f) * halfWidth;
			vertex.position.y = (1.f - vertex.position.y) * halfHeight;

			++i;
		}
	}
}

void Renderer::CalculateBoundingBox(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Int2& min, Int2& max) const
{
	// Compute the minimum and maximum x and y coordinates of the triangle.
	min.x = static_cast<int>(std::floor(std::max(.0f, std::min(v0.position.x, std::min(v1.position.x, v2.position.x)))));
	min.y = static_cast<int>(std::floor(std::max(.0f, std::min(v0.position.y, std::min(v1.position.y, v2.position.y)))));
	max.x = static_cast<int>(std::ceil(std::min(m_fWidth - 1.f, std::max(v0.position.x, std::max(v1.position.x, v2.position.x)))));
	max.y = static_cast<int>(std::ceil(std::min(m_fHeight - 1.f, std::max(v0.position.y, std::max(v1.position.y, v2.position.y)))));
}

void Renderer::InitializeMesh(const char* path, const Matrix& worldMatrix, const PrimitiveTopology& topology)
{
	m_Meshes.emplace_back();
	m_Meshes.back().primitiveTopology = topology;
	m_Meshes.back().worldMatrix = worldMatrix;
	Utils::ParseOBJ(path, m_Meshes.back().vertices, m_Meshes.back().indices);

	VertexTransformationFunction(m_Meshes);
}

bool Renderer::IsOutsideViewFrustum(const Vertex_Out& v) const
{
	return
		v.position.x < .0f || v.position.x > m_fWidth ||
		v.position.y < .0f || v.position.y > m_fHeight ||
		v.position.z < .0f || v.position.z > 1.f;
}

void Renderer::RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const
{
	if (IsOutsideViewFrustum(v0) || IsOutsideViewFrustum(v1) || IsOutsideViewFrustum(v2)) return;

	const Vector2& v0Pos{ v0.position.GetXY() };
	const Vector2& v1Pos{ v1.position.GetXY() };
	const Vector2& v2Pos{ v2.position.GetXY() };

	// Calculate the bounding box - but make sure the triangle is inside the screen
	Int2 min;
	Int2 max;
	CalculateBoundingBox(v0, v1, v2, min, max);

	const float area{ EdgeFunction(v0Pos, v1Pos, v2Pos) };

	if (area < FLT_EPSILON) return;

	const float invArea{ Inverse(area) };

	// Pre-calculate the inverse z
	const float z0{ Inverse(v0.position.z) };
	const float z1{ Inverse(v1.position.z) };
	const float z2{ Inverse(v2.position.z) };

	// Pre-calculate the inverse w
	const float w0V{ Inverse(v0.position.w) };
	const float w1V{ Inverse(v1.position.w) };
	const float w2V{ Inverse(v2.position.w) };

	// Pre calculate the uv coordinates
	const Vector2& uv0{ v0.uv / v0.position.w };
	const Vector2& uv1{ v1.uv / v1.position.w };
	const Vector2& uv2{ v2.uv / v2.position.w };

	// Pre calculate the color coordinates
	const ColorRGB& c0{ v0.color / v0.position.w };
	const ColorRGB& c1{ v1.color / v1.position.w };
	const ColorRGB& c2{ v2.color / v2.position.w };

	// Loop over the bounding box
	for (int py{ min.y }; py < max.y; ++py)
	{
		for (int px{ min.x }; px < max.x; ++px)
		{
			// Check if the pixel is inside the triangle
			// If so, draw the pixel
			const Vector2 pixel{ static_cast<float>(px) + .5f, static_cast<float>(py) + .5f };

			const float w0{ EdgeFunction(v1Pos, v2Pos, pixel) * invArea };
			if (w0 < .0f) continue;

			const float w1{ EdgeFunction(v2Pos, v0Pos, pixel) * invArea };
			if (w1 < .0f) continue;

			// Optimize by not calculating the cross product for the last edge
			const float w2{ 1.f - w0 - w1 };
			if (w2 < .0f) continue;

			// Calculate the depth account for perspective interpolation
			const float z{ Inverse(z0 * w0 + z1 * w1 + z2 * w2) };
			const int zBufferIdx{ py * m_Width + px };
			float& zBuffer{ m_pDepthBufferPixels[zBufferIdx] };

			//Check if pixel is in front of the current pixel in the depth buffer
			if (z >= zBuffer) continue;

			//Update depth buffer
			zBuffer = z;

			ColorRGB finalColor{ colors::Black };

			if (m_RenderDepthBuffer)
			{
				const float depthColor{ Remap(z, .997f, 1.f) };
				finalColor = colors::White * depthColor;
			}
			else
			{
				// Interpolated w
				const float w{ Inverse(w0V * w0 + w1V * w1 + w2V * w2) };

				const Vertex_Out interpolatedVertex
				{
					{ pixel.x, pixel.y, z, w },
					c0 * w0 + c1 * w1 + c2 * w2,
					(uv0 * w0 + uv1 * w1 + uv2 * w2) * w,
					((v0.normal * w0 + v1.normal * w1 + v2.normal * w2) * w).Normalized(),
					((v0.tangent * w0 + v1.tangent * w1 + v2.tangent * w2) * w).Normalized(),
					((v0.viewDirection * w0 + v1.viewDirection * w1 + v2.viewDirection * w2) * w).Normalized()
				};

				finalColor = PixelShading(interpolatedVertex);
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[zBufferIdx] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

float Renderer::EdgeFunction(const Vector2& a, const Vector2& b, const Vector2& c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

ColorRGB Renderer::PixelShading(const Vertex_Out& v) const
{
	// Sampled texture colors
	const ColorRGB sampledColor{ m_pTexture->Sample(v.uv) };
	const ColorRGB sampledGloss{ m_pGlossMap->Sample(v.uv) };
	const ColorRGB sampledNormal{ m_pNormalMap->Sample(v.uv) };
	const ColorRGB sampledSpecular{ m_pSpecularMap->Sample(v.uv) };

	// Normal mapping
	const Vector3 binormal{ Vector3::Cross(v.normal, v.tangent) };
	const Matrix tangentSpaceAxis{ v.tangent, binormal, v.normal, Vector3::Zero };
	const Vector3 normal{ m_RenderNormalMap ? (tangentSpaceAxis.TransformVector(2.f * sampledNormal.ToVector3() - Vector3::One).Normalized()) : v.normal };

	const ColorRGB diffuse{ m_LightIntensity * sampledColor / PI };

	const float observedArea{ Vector3::Dot(normal, -m_LightDirection) };
	if (observedArea < .0f) return colors::Black;

	// Calculate diffuse and specular lighting
	const float exp{ sampledGloss.r * m_Shininess };
	const ColorRGB specular{ (colors::White * sampledSpecular * powf(std::max(Vector3::Dot(Vector3::Reflect(-m_LightDirection, normal), v.viewDirection), .0f), exp)) };

	switch (m_CurrentShadingMode)
	{
		using enum ShadingMode;
	case ObservedArea:
		return { observedArea, observedArea, observedArea };
	case Diffuse:
		return observedArea * diffuse;
	case Specular:
		return specular;
	case Combined:
		return (diffuse + specular + m_Ambient) * observedArea;
	}

	return colors::Black;
}

void Renderer::RenderMesh(const Mesh& mesh) const
{
	const bool isTriangleList{ mesh.primitiveTopology == PrimitiveTopology::TriangleList };

	const int increment{ isTriangleList ? 3 : 1 };
	const size_t size{ isTriangleList ? mesh.indices.size() : mesh.indices.size() - 2 };

	for (int i{ 0 }; i < size; i += increment)
	{
		const uint32_t& idx0{ mesh.indices[i] };
		const uint32_t& idx1{ mesh.indices[i + 1] };
		const uint32_t& idx2{ mesh.indices[i + 2] };

		// If any of the indexes are equal skip
		if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0) continue;

		const Vertex_Out& v0{ mesh.vertices_out[idx0] };
		const Vertex_Out& v1{ mesh.vertices_out[idx1] };
		const Vertex_Out& v2{ mesh.vertices_out[idx2] };

		if (isTriangleList)
		{
			RenderTriangle(v0, v1, v2);
			continue;
		}
		RenderTriangle(i % 2 == 0 ? v0 : v2, v1, i % 2 == 0 ? v2 : v0);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
