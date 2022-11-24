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
	m_MeshRotationAngle{ 45.f },
	m_pTexture{ Texture::LoadFromFile("Resources/tuktuk.png") }
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
	m_Camera.Initialize(60.f, { .0f, .0f, -10.f }, m_AspectRatio);

	const Vector3 position{ m_Camera.origin + Vector3{ .0f, -5.f, 25.f } };
	InitializeMesh("Resources/tuktuk.obj", Matrix::CreateTranslation(position));
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

	//Render_W3(); //Matrix Transformations
	Render_Tuktuk();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, nullptr, m_pFrontBuffer, nullptr);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::Render_Tuktuk()
{
	ClearBuffers();
	VertexTransformationFunction(m_Meshes);
	RenderMesh(m_Meshes[0], m_pTexture);
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
			Vector4& vertexPos{ mesh.vertices_out[i].position };
			vertexPos = viewProjectionMatrix.TransformPoint({ mesh.vertices[i].position, 1.f });

			vertexPos.x /= vertexPos.w;
			vertexPos.y /= vertexPos.w;
			vertexPos.z /= vertexPos.w;

			vertexPos.x = (vertexPos.x + 1.f) * halfWidth;
			vertexPos.y = (1.f - vertexPos.y) * halfHeight;
		}
	}
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
		v.position.x < .0f ||
		v.position.x > m_fWidth ||
		v.position.y < 0.f ||
		v.position.y > m_fHeight ||
		v.position.z < .0f ||
		v.position.z > 1.f;
}

void Renderer::RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const Texture* pTexture) const
{
	if (IsOutsideViewFrustum(v0) || IsOutsideViewFrustum(v1) || IsOutsideViewFrustum(v2)) return;

	// Create aliases for the vertex positions
	const Vector4& v0Pos{ v0.position };
	const Vector4& v1Pos{ v1.position };
	const Vector4& v2Pos{ v2.position };

	// Check if the triangle is behind the camera by sign checking
	//if (v0Pos.w < .0f || v1Pos.w < .0f || v2Pos.w < .0f) return;

	//Pre-calculate dimensions
	const float width{ m_fWidth - 1.f };
	const float height{ m_fHeight - 1.f };

	// Calculate the bounding box - but make sure the triangle is inside the screen
	const int minX{ static_cast<int>(std::floor(std::max(.0f, std::min(v0Pos.x, std::min(v1Pos.x, v2Pos.x))))) };
	const int maxX{ static_cast<int>(std::ceil(std::min(width, std::max(v0Pos.x, std::max(v1Pos.x, v2Pos.x))))) };
	const int minY{ static_cast<int>(std::floor(std::max(.0f, std::min(v0Pos.y, std::min(v1Pos.y, v2Pos.y))))) };
	const int maxY{ static_cast<int>(std::ceil(std::min(height, std::max(v0Pos.y, std::max(v1Pos.y, v2Pos.y))))) };

	// is triangle off screen?
	if (minX > maxX || minY > maxY) return;

	const float area
	{ Vector2::Cross(
		{ v1Pos.x - v0Pos.x, v1Pos.y - v0Pos.y },
		{ v2Pos.x - v0Pos.x, v2Pos.y - v0Pos.y })
	};

	if (area < FLT_EPSILON) return;

	const float invArea{ 1.f / area };

	// Pre-calculate the inverse z
	const float z0{ 1.f / v0Pos.z };
	const float z1{ 1.f / v1Pos.z };
	const float z2{ 1.f / v2Pos.z };

	// Pre-calculate the inverse w
	const float w0V{ 1.f / v0Pos.w };
	const float w1V{ 1.f / v1Pos.w };
	const float w2V{ 1.f / v2Pos.w };

	// Pre calculate the uv coordinates
	const Vector2 uv0{ v0.uv / v0Pos.w };
	const Vector2 uv1{ v1.uv / v1Pos.w };
	const Vector2 uv2{ v2.uv / v2Pos.w };

	// Pre calculate the color coordinates
	const ColorRGB c0{ v0.color / v0Pos.w };
	const ColorRGB c1{ v1.color / v1Pos.w };
	const ColorRGB c2{ v2.color / v2Pos.w };

	// Loop over the bounding box
	for (int py{ minY }; py < maxY; ++py)
	{
		for (int px{ minX }; px < maxX; ++px)
		{
			// Check if the pixel is inside the triangle
			// If so, draw the pixel
			const Vector2 pixel{ static_cast<float>(px) + .5f, static_cast<float>(py) + .5f };

			float w0
			{ Vector2::Cross(
				{ pixel.x - v1Pos.x, pixel.y - v1Pos.y },
				{ pixel.x - v2Pos.x, pixel.y - v2Pos.y })
			};
			if (w0 < 0) continue;

			float w1
			{ Vector2::Cross(
				{ pixel.x - v2Pos.x, pixel.y - v2Pos.y },
				{ pixel.x - v0Pos.x, pixel.y - v0Pos.y })
			};
			if (w1 < 0) continue;

			float w2{ area - w0 - w1 };
			if (w2 < 0) continue;

			w0 *= invArea;
			w1 *= invArea;
			w2 *= invArea;

			// Calculate the depth account for perspective interpolation
			const float z{ 1.f / (z0 * w0 + z1 * w1 + z2 * w2) };
			const int zBufferIdx{ py * m_Width + px };
			float& zBuffer{ m_pDepthBufferPixels[zBufferIdx] };

			//Check if pixel is in front of the current pixel in the depth buffer
			if (z > zBuffer)
				continue;

			//Update depth buffer
			zBuffer = z;

			// Interpolated w
			const float w{ 1.f / (w0V * w0 + w1V * w1 + w2V * w2) };

			ColorRGB finalColor{};

			if (pTexture && !m_RenderDepthBuffer)
			{
				// Interpolate uv coordinates correct for perspective
				const Vector2 uv{ (uv0 * w0 + uv1 * w1 + uv2 * w2) * w };

				// Sample the texture
				finalColor = pTexture->Sample(uv);
			}
			else if (m_RenderDepthBuffer)
			{
				const float depthColor{ Remap(z, .985f, 1.f) };
				finalColor = { depthColor, depthColor, depthColor };
			}
			else
			{
				// Interpolate color correct for perspective
				finalColor = (c0 * w0 + c1 * w1 + c2 * w2) * w;
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