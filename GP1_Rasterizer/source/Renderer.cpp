//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow{ pWindow },
	m_MeshRotationAngle{ 57.5f }
	//m_pTexture{ Texture::LoadFromFile("Resources/vehicle_diffuse.png") }
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
	m_Camera.Initialize(45.f, { .0f, .0f, .0f }, m_AspectRatio);

	const Vector3 position{ .0f, .0f, 50.f };
	constexpr float angle{ 90.f * TO_RADIANS };
	const Matrix worldMatrix{ Matrix::CreateRotationY(angle) * Matrix::CreateTranslation(position) };
	InitializeMesh("Resources/vehicle.obj", worldMatrix);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	m_pDepthBufferPixels = nullptr;

	if (m_pTexture)
	{
		delete m_pTexture;
		m_pTexture = nullptr;
	}
}

void Renderer::Update(const Timer* pTimer)
{
	m_Camera.Update(pTimer);

	//for (Mesh& mesh : m_Meshes)
	//{
	//	mesh.RotateY(m_MeshRotationAngle * pTimer->GetElapsed());
	//}
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	ClearBuffers(100, 100, 100);
	VertexTransformationFunction(m_Meshes);
	RenderMesh(m_Meshes[0], m_pTexture);

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

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes) const
{
	Matrix viewProjectionMatrix{ m_Camera.viewMatrix * m_Camera.projectionMatrix };

	const float halfWidth{ m_fWidth * .5f };
	const float halfHeight{ m_fHeight * .5f };

	for (Mesh& mesh : meshes)
	{
		viewProjectionMatrix = mesh.worldMatrix * viewProjectionMatrix;

		if (mesh.vertices_out.empty())
		{
			mesh.vertices_out.reserve(mesh.vertices.size());
			for (const Vertex& vertex : mesh.vertices)
			{
				mesh.vertices_out.push_back({ {}, vertex.color, vertex.uv, vertex.normal, vertex.tangent });
			}
		}

		for (int i{ 0 }; i < mesh.vertices.size(); ++i)
		{
			Vertex_Out& vertex{ mesh.vertices_out[i] };
			vertex.position = viewProjectionMatrix.TransformPoint({ mesh.vertices[i].position, 1.f });

			vertex.normal = mesh.worldMatrix.TransformVector(mesh.vertices[i].normal).Normalized();

			vertex.position.x /= vertex.position.w;
			vertex.position.y /= vertex.position.w;
			vertex.position.z /= vertex.position.w;

			vertex.position.x = (vertex.position.x + 1.f) * halfWidth;
			vertex.position.y = (1.f - vertex.position.y) * halfHeight;
		}
	}
}

void Renderer::CalculateBoundingBox(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Int2& min, Int2& max) const
{
	// Returns false if triangle is degenerate
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
	path = nullptr;

	VertexTransformationFunction(m_Meshes);
}

bool Renderer::IsOutsideViewFrustum(const Vertex_Out& v) const
{
	return
		(v.position.x < .0f || v.position.x > m_fWidth) ||
		(v.position.y < .0f || v.position.y > m_fHeight) ||
		(v.position.z < .0f || v.position.z > 1.f);
}

void Renderer::RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const Texture* pTexture) const
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
			if (z > zBuffer) continue;

			//Update depth buffer
			zBuffer = z;

			// Interpolated w
			const float w{ Inverse(w0V * w0 + w1V * w1 + w2V * w2) };

			const Vertex_Out fragmentToShade
			{
				{ pixel.x, pixel.y, z, w },
				c0 * w0 + c1 * w1 + c2 * w2,
				uv0 * w0 + uv1 * w1 + uv2 * w2,
				(v0.normal * w0 + v1.normal * w1 + v2.normal * w2) * w,
				(v0.tangent * w0 + v1.tangent * w1 + v2.tangent * w2) * w,
			};

			ColorRGB finalColor{ PixelShading(fragmentToShade) };

			//if (pTexture && !m_RenderDepthBuffer)
			//{
			//	// Interpolate uv coordinates correct for perspective
			//	const Vector2 uv{ (uv0 * w0 + uv1 * w1 + uv2 * w2) * w };

			//	// Sample the texture
			//	finalColor = pTexture->Sample(uv);
			//}
			//else if (m_RenderDepthBuffer)
			//{
			//	const float depthColor{ Remap(z, .985f, 1.f) };
			//	finalColor = { depthColor, depthColor, depthColor };
			//}
			//else
			//{
			//	// Interpolate color correct for perspective
			//	finalColor = (c0 * w0 + c1 * w1 + c2 * w2) * w;
			//}

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
	static const Vector3 lightDirection{ .577f, -.577f, .577f };
	const float diffuse{ std::max(0.f, Vector3::Dot(v.normal, -lightDirection)) };
	return v.color * diffuse;
}

void Renderer::RenderMesh(const Mesh& mesh, const Texture* pTexture) const
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
			RenderTriangle(v0, v1, v2, pTexture);
			continue;
		}
		RenderTriangle(i % 2 == 0 ? v0 : v2, v1, i % 2 == 0 ? v2 : v0, pTexture);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}