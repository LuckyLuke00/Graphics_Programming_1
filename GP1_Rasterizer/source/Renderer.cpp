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
	m_pTexture{ Texture::LoadFromFile("Resources/uv_grid_2.png") }

{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
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

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//Render_W1_Part1(); //Rasterizer Stage Only
	//Render_W1_Part2(); //Projection Stage (Camera)
	//Render_W1_Part3(); //Barycentric Coordinates
	//Render_W1_Part4(); //Depth Buffer
	//Render_W1_Part5(); //BoundingBox Optimization

	Render_W2(); //Textures

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::Render_W1_Part1()
{
	//Define Triangle - Vertices in NDC space
	static const std::vector<Vector3> vertices_ndc
	{
		{  .0f,  .5f, 1.f },
		{  .5f, -.5f, 1.f },
		{ -.5f, -.5f, 1.f },
	};

	static const float halfHeight{ m_Height * .5f };
	static const float halfWidth{ m_Width * .5f };

	//Define Triangle - Vertices in SCREEN space
	static const std::vector<Vector3> vertices_screen
	{
		{ (vertices_ndc[0].x + 1.f) * halfWidth, (1.f - vertices_ndc[0].y) * halfHeight, vertices_ndc[0].z },
		{ (vertices_ndc[1].x + 1.f) * halfWidth, (1.f - vertices_ndc[1].y) * halfHeight, vertices_ndc[1].z },
		{ (vertices_ndc[2].x + 1.f) * halfWidth, (1.f - vertices_ndc[2].y) * halfHeight, vertices_ndc[2].z },
	};

	//RENDER LOGIC
	for (int px{ 0 }; px < m_Width; ++px)
	{
		for (int py{ 0 }; py < m_Height; ++py)
		{
			// This time you don�t do the inside-outside test with an intersection point, but with the
			// pixel itself! So, define the pixel as an 2D point : Vector2(px, py). Of course, don�t use the
			// pixel directly for the cross products.Remember we need the vector pointing from the
			// current vertex to the pixel!

			// One big if statement that checks if the pixel is inside the triangle.
			if (const Vector2 pixel{ px + .5f, py + .5f };
				Vector2::Cross(
					{ pixel.x - vertices_screen[0].x, pixel.y - vertices_screen[0].y },
					{ pixel.x - vertices_screen[1].x, pixel.y - vertices_screen[1].y }) < .0f ||
				Vector2::Cross(
					{ pixel.x - vertices_screen[1].x, pixel.y - vertices_screen[1].y },
					{ pixel.x - vertices_screen[2].x, pixel.y - vertices_screen[2].y }) < .0f ||
				Vector2::Cross(
					{ pixel.x - vertices_screen[2].x, pixel.y - vertices_screen[2].y },
					{ pixel.x - vertices_screen[0].x, pixel.y - vertices_screen[0].y }) < .0f

				) continue;

			ColorRGB finalColor{ colors::White };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void dae::Renderer::Render_W1_Part2()
{
	//Define Triangle - Vertices in WORLD space
	static const std::vector<Vertex> vertices_world
	{
		{ {  .0f, 2.f, .0f } },
		{ {  1.f, .0f, .0f } },
		{ { -1.f, .0f, .0f } },
	};

	static std::vector<Vertex> vertices_screen;
	VertexTransformationFunction(vertices_world, vertices_screen);

	// Clear the back buffer
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 0, 0, 0));

	//RENDER LOGIC
	for (int px{ 0 }; px < m_Width; ++px)
	{
		for (int py{ 0 }; py < m_Height; ++py)
		{
			if (const Vector2 pixel{ px + .5f, py + .5f };
				Vector2::Cross(
					{ pixel.x - vertices_screen[0].position.x, pixel.y - vertices_screen[0].position.y },
					{ pixel.x - vertices_screen[1].position.x, pixel.y - vertices_screen[1].position.y }) < .0f ||
				Vector2::Cross(
					{ pixel.x - vertices_screen[1].position.x, pixel.y - vertices_screen[1].position.y },
					{ pixel.x - vertices_screen[2].position.x, pixel.y - vertices_screen[2].position.y }) < .0f ||
				Vector2::Cross(
					{ pixel.x - vertices_screen[2].position.x, pixel.y - vertices_screen[2].position.y },
					{ pixel.x - vertices_screen[0].position.x, pixel.y - vertices_screen[0].position.y }) < .0f
				) continue;

			ColorRGB finalColor{ colors::White };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void dae::Renderer::Render_W1_Part3()
{
	//Define Triangle - Vertices in WORLD space
	static const std::vector<Vertex> vertices_world
	{
		{ {  .0f,  4.f, 2.f }, { 1.f, .0f, .0f } },
		{ {  3.f, -2.f, 2.f }, { .0f, 1.f, .0f } },
		{ { -3.f, -2.f, 2.f }, { .0f, .0f, 1.f } },
	};

	static std::vector<Vertex> vertices_screen;
	VertexTransformationFunction(vertices_world, vertices_screen);

	// Clear the back buffer
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 0, 0, 0));

	const float area
	{ Vector2::Cross(
		{ vertices_screen[1].position.x - vertices_screen[0].position.x, vertices_screen[1].position.y - vertices_screen[0].position.y },
		{ vertices_screen[2].position.x - vertices_screen[0].position.x, vertices_screen[2].position.y - vertices_screen[0].position.y })
	};

	//RENDER LOGIC
	for (int px{ 0 }; px < m_Width; ++px)
	{
		for (int py{ 0 }; py < m_Height; ++py)
		{
			const Vector2 pixel{ px + .5f, py + .5f };

			float w0
			{ Vector2::Cross(
				{ pixel.x - vertices_screen[0].position.x, pixel.y - vertices_screen[0].position.y },
				{ pixel.x - vertices_screen[1].position.x, pixel.y - vertices_screen[1].position.y })
			};
			if (w0 < .0f) continue;

			float w1
			{ Vector2::Cross(
				{ pixel.x - vertices_screen[1].position.x, pixel.y - vertices_screen[1].position.y },
				{ pixel.x - vertices_screen[2].position.x, pixel.y - vertices_screen[2].position.y })
			};
			if (w1 < .0f) continue;

			float w2{ area - w0 - w1 };
			if (w2 < .0f) continue;

			w0 /= area;
			w1 /= area;
			w2 /= area;

			//Interpolate color
			ColorRGB finalColor{ (vertices_screen[0].color * w0) + (vertices_screen[1].color * w1) + (vertices_screen[2].color * w2) };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void dae::Renderer::Render_W1_Part4()
{
	//Define Triangle - Vertices in WORLD space
	static const std::vector<Vertex> vertices_world
	{
		//Triangle 0
		{ {   .0f,  2.f, .0f }, { 1.f, .0f, .0f } },
		{ {  1.5f, -1.f, .0f }, { 1.f, .0f, .0f } },
		{ { -1.5f, -1.f, .0f }, { 1.f, .0f, .0f } },

		//Triangle 1
		{ {  .0f,  4.f, 2.f }, { 1.f, .0f, .0f } },
		{ {  3.f, -2.f, 2.f }, { .0f, 1.f, .0f } },
		{ { -3.f, -2.f, 2.f }, { .0f, .0f, 1.f } },
	};

	static std::vector<Vertex> vertices_screen;
	VertexTransformationFunction(vertices_world, vertices_screen);

	// Initialize the depth buffer to float max
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	// Also clear the BackBuffer (SDL_FillRect, clearColor = {100,100,100}
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//RENDER LOGIC
	// For every triangle
	for (size_t i{}; i < vertices_screen.size(); i += 3)
	{
		const float area
		{ Vector2::Cross(
			{ vertices_screen[i + 1].position.x - vertices_screen[i + 0].position.x, vertices_screen[i + 1].position.y - vertices_screen[i + 0].position.y },
			{ vertices_screen[i + 2].position.x - vertices_screen[i + 0].position.x, vertices_screen[i + 2].position.y - vertices_screen[i + 0].position.y })
		};

		for (int px{ 0 }; px < m_Width; ++px)
		{
			for (int py{ 0 }; py < m_Height; ++py)
			{
				const Vector2 pixel{ px + .5f, py + .5f };

				float w0
				{ Vector2::Cross(
					{ pixel.x - vertices_screen[i + 0].position.x, pixel.y - vertices_screen[i + 0].position.y },
					{ pixel.x - vertices_screen[i + 1].position.x, pixel.y - vertices_screen[i + 1].position.y })
				};
				if (w0 < .0f) continue;

				float w1
				{ Vector2::Cross(
					{ pixel.x - vertices_screen[i + 1].position.x, pixel.y - vertices_screen[i + 1].position.y },
					{ pixel.x - vertices_screen[i + 2].position.x, pixel.y - vertices_screen[i + 2].position.y })
				};
				if (w1 < .0f) continue;

				float w2{ area - w0 - w1 };
				if (w2 < .0f) continue;

				w0 /= area;
				w1 /= area;
				w2 /= area;

				//Interpolate z
				const float z{ (vertices_screen[i + 0].position.z * w0) + (vertices_screen[i + 1].position.z * w1) + (vertices_screen[i + 2].position.z * w2) };

				//Check if pixel is in front of the current pixel in the depth buffer
				// Calculate buffer index
				const int bufferIdx{ px + (py * m_Width) };

				if (z > m_pDepthBufferPixels[bufferIdx])
					continue;

				//Update depth buffer
				m_pDepthBufferPixels[bufferIdx] = z;

				//Interpolate color
				ColorRGB finalColor{ (vertices_screen[i + 0].color * w0) + (vertices_screen[i + 1].color * w1) + (vertices_screen[i + 2].color * w2) };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[bufferIdx] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

void dae::Renderer::Render_W1_Part5()
{
	//Define Triangle - Vertices in WORLD space
	static const std::vector<Vertex> vertices_world
	{
		//Triangle 0
		{{ .0f, 2.f, .0f }, { 1.f, .0f, .0f }},
		{{ 1.5f, -1.f, .0f }, { 1.f, .0f, .0f }},
		{{ -1.5f, -1.f, .0f }, { 1.f, .0f, .0f }},

		//Triangle 1
		{{ .0f, 4.f, 2.f }, { 1.f, .0f, .0f }},
		{{ 3.f, -2.f, 2.f }, { .0f, 1.f, .0f }},
		{{ -3.f, -2.f, 2.f }, { .0f, .0f, 1.f }},
	};

	static std::vector<Vertex> vertices_screen;
	VertexTransformationFunction(vertices_world, vertices_screen);

	// Initialize the depth buffer to float max
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	// Also clear the BackBuffer (SDL_FillRect, clearColor = {100,100,100}
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//RENDER LOGIC
	// For every triangle
	for (size_t i{}; i < vertices_screen.size(); i += 3)
	{
		RenderTriangle(vertices_screen[i + 0], vertices_screen[i + 1], vertices_screen[i + 2]);
	}
}

void dae::Renderer::Render_W2()
{
	//Define Mesh - Triangle List
	//const static std::vector<Mesh> meshes_world
	//{
	//	Mesh
	//	{
	//		{
	//			Vertex{ { -3,  3, -2 }, { colors::White }, { .0f, .0f } },
	//			Vertex{ {  0,  3, -2 }, { colors::White }, { .5f, .0f } },
	//			Vertex{ {  3,  3, -2 }, { colors::White }, { 1.f, .0f } },
	//			Vertex{ { -3,  0, -2 }, { colors::White }, { .0f, .5f } },
	//			Vertex{ {  0,  0, -2 }, { colors::White }, { .5f, .5f } },
	//			Vertex{ {  3,  0, -2 }, { colors::White }, { 1.f, .5f } },
	//			Vertex{ { -3, -3, -2 }, { colors::White }, { .0f, 1.f } },
	//			Vertex{ {  0, -3, -2 }, { colors::White }, { .5f, 1.f } },
	//			Vertex{ {  3, -3, -2 }, { colors::White }, { 1.f, 1.f } },
	//		},
	//		{
	//			3, 0, 1,	1, 4, 3,	4, 1, 2,
	//			2, 5, 4,	6, 3, 4,	4, 7, 6,
	//			7, 4, 5,	5, 8, 7
	//		},
	//		PrimitiveTopology::TriangleList
	//	}
	//};

	//Define Mesh - Triangle Strip
	const static std::vector<Mesh> meshes_world
	{
		Mesh
		{
			{
				Vertex{ { -3,  3, -2 }, { colors::White }, { .0f, .0f } },
				Vertex{ {  0,  3, -2 }, { colors::White }, { .5f, .0f } },
				Vertex{ {  3,  3, -2 }, { colors::White }, { 1.f, .0f } },
				Vertex{ { -3,  0, -2 }, { colors::White }, { .0f, .5f } },
				Vertex{ {  0,  0, -2 }, { colors::White }, { .5f, .5f } },
				Vertex{ {  3,  0, -2 }, { colors::White }, { 1.f, .5f } },
				Vertex{ { -3, -3, -2 }, { colors::White }, { .0f, 1.f } },
				Vertex{ {  0, -3, -2 }, { colors::White }, { .5f, 1.f } },
				Vertex{ {  3, -3, -2 }, { colors::White }, { 1.f, 1.f } },
			},
			{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},
		PrimitiveTopology::TriangleStrip
		}
	};

	static std::vector<Mesh> meshes_screen;
	VertexTransformationFunction(meshes_world, meshes_screen);

	// Initialize the depth buffer to float max
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	// Also clear the BackBuffer (SDL_FillRect, clearColor = {100,100,100}
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	// According to the PrimitiveTopology, use a different index loop
	for (const Mesh& mesh : meshes_screen)
	{
		if (mesh.primitiveTopology == PrimitiveTopology::TriangleList)
		{
			for (size_t i{ 0 }; i < mesh.indices.size(); i += 3)
			{
				const Vertex& v0{ mesh.vertices[mesh.indices[i + 0]] };
				const Vertex& v1{ mesh.vertices[mesh.indices[i + 1]] };
				const Vertex& v2{ mesh.vertices[mesh.indices[i + 2]] };

				RenderTriangle(v0, v1, v2, m_pTexture);
			}
			continue;
		}
		// Change your index loop accordingly(not pixel loop!). Considering if it�s an odd or even triangle if
		// using the triangle strip technique.Hint: odd or even ? -> modulo or bit masking

		for (size_t i{ 0 }; i < mesh.indices.size() - 2; ++i)
		{
			if (i + 2 > mesh.indices.size()) break;

			const Vertex& v0{ mesh.vertices[mesh.indices[i + 0]] };
			const Vertex& v1{ mesh.vertices[mesh.indices[i + 1]] };
			const Vertex& v2{ mesh.vertices[mesh.indices[i + 2]] };

			if (i % 2 == 0)
			{
				RenderTriangle(v0, v1, v2, m_pTexture);
				continue;
			}

			RenderTriangle(v0, v2, v1, m_pTexture);
		}
	}
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	// Only copy and reserve space for meshes out once
	if (vertices_out.empty())
	{
		vertices_out.reserve(vertices_in.size());
		vertices_out = vertices_in;
	}

	// Convert with and height to float
	const float width{ static_cast<float>(m_Width) };
	const float height{ static_cast<float>(m_Height) };

	const float aspectRatioFov{ width / height * m_Camera.fov };
	const float fovReciprocal{ 1.f / m_Camera.fov };

	for (int i{ 0 }; i < vertices_out.size(); ++i)
	{
		const Vector3 viewSpacePos{ m_Camera.viewMatrix.TransformPoint(vertices_in[i].position) };
		Vector3& vertexPos{ vertices_out[i].position };

		vertexPos.x = (viewSpacePos.x / viewSpacePos.z / aspectRatioFov + 1.f) * (width * .5f);
		vertexPos.y = (1.f - viewSpacePos.y / viewSpacePos.z * fovReciprocal) * (height * .5f);
		vertexPos.z = viewSpacePos.z;
	}
}
void Renderer::VertexTransformationFunction(const std::vector<Mesh>& meshes_in, std::vector<Mesh>& meshes_out) const //W2 Version
{
	// Only copy and reserve space for meshes out once
	if (meshes_out.empty())
	{
		meshes_out.reserve(meshes_in.size() * 3);
		meshes_out = meshes_in;
	}

	// Convert with and height to float
	const float width{ static_cast<float>(m_Width) };
	const float height{ static_cast<float>(m_Height) };

	const float aspectRatioFov{ width / height * m_Camera.fov };
	const float fovReciprocal{ 1.f / m_Camera.fov };

	for (int i{ 0 }; i < meshes_out.size(); ++i)
	{
		for (int j{ 0 }; j < meshes_out[i].vertices.size(); ++j)
		{
			const Vector3 viewSpacePos{ m_Camera.viewMatrix.TransformPoint(meshes_in[i].vertices[j].position) };
			Vector3& vertexPos{ meshes_out[i].vertices[j].position };

			vertexPos.x = (viewSpacePos.x / viewSpacePos.z / aspectRatioFov + 1.f) * (width * .5f);
			vertexPos.y = (1.f - viewSpacePos.y / viewSpacePos.z * fovReciprocal) * (height * .5f);
			vertexPos.z = viewSpacePos.z;
		}
	}
}

void dae::Renderer::RenderTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Texture* pTexture) const
{
	// Create aliases for the vertex positions
	const Vector3& v0Pos{ v0.position };
	const Vector3& v1Pos{ v1.position };
	const Vector3& v2Pos{ v2.position };

	// Check if the triangle is behind the camera by sign checking
	if (v0Pos.z < .0f || v1Pos.z < .0f || v2Pos.z < .0f) return;

	//Pre-calculate dimentions
	const float width{ m_Width - 1.f };
	const float height{ m_Height - 1.f };

	// Calculate the bounding box - but make sure the triangle is inside the screen
	const int minX{ static_cast<int>(std::floor(std::max(.0f, std::min(v0Pos.x, std::min(v1Pos.x, v2Pos.x))))) };
	const int maxX{ static_cast<int>(std::ceil(std::min(width, std::max(v0Pos.x, std::max(v1Pos.x, v2Pos.x))))) };
	const int minY{ static_cast<int>(std::floor(std::max(.0f, std::min(v0Pos.y, std::min(v1Pos.y, v2Pos.y))))) };
	const int maxY{ static_cast<int>(std::ceil(std::min(height, std::max(v0Pos.y, std::max(v1Pos.y, v2Pos.y))))) };

	// is triangle of screen?
	if (minX > maxX || minY > maxY) return;

	const float area
	{ Vector2::Cross(
		{ v1Pos.x - v0Pos.x, v1Pos.y - v0Pos.y },
		{ v2Pos.x - v0Pos.x, v2Pos.y - v0Pos.y })
	};

	if (area < FLT_EPSILON) return;

	// Pre-caclulate the inverse area
	const float z0{ 1.f / v0Pos.z };
	const float z1{ 1.f / v1Pos.z };
	const float z2{ 1.f / v2Pos.z };

	// Pre calculate the uv coordinates
	const Vector2 uv0{ v0.uv / v0Pos.z };
	const Vector2 uv1{ v1.uv / v1Pos.z };
	const Vector2 uv2{ v2.uv / v2Pos.z };

	// Pre calculate the color coordinates
	const ColorRGB c0{ v0.color / v0Pos.z };
	const ColorRGB c1{ v1.color / v1Pos.z };
	const ColorRGB c2{ v2.color / v2Pos.z };

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

			// Calculate the depth account for perspective interpolation
			const float z{ 1.f / ((z0 * w0 + z1 * w1 + z2 * w2) / area) };
			const int zBufferIdx{ py * m_Width + px };
			float& zBuffer{ m_pDepthBufferPixels[zBufferIdx] };

			//Check if pixel is in front of the current pixel in the depth buffer
			if (z > zBuffer)
				continue;

			//Update depth buffer
			zBuffer = z;

			w0 /= area;
			w1 /= area;
			w2 /= area;

			ColorRGB finalColor{};

			if (pTexture)
			{
				// Interpolate uv coordinates correct for perspective
				const Vector2 uv{ (uv0 * w0 + uv1 * w1 + uv2 * w2) * z };

				// Sample the texture
				finalColor = pTexture->Sample(uv);
			}
			else
			{
				// Interpolate color correct for perspective
				finalColor = (c0 * w0 + c1 * w1 + c2 * w2) * z;
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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}