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
	Render_W1_Part4(); //Depth Buffer
	//Render_W1_Part5(); //BoundingBox Optimization

	//Render_W2(); //Textures

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
			// This time you don’t do the inside-outside test with an intersection point, but with the
			// pixel itself! So, define the pixel as an 2D point : Vector2(px, py). Of course, don’t use the
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
		// Find max and min of the triangle
		const float minX{ std::min(vertices_screen[i].position.x, std::min(vertices_screen[i + 1].position.x, vertices_screen[i + 2].position.x)) };
		const float maxX{ std::max(vertices_screen[i].position.x, std::max(vertices_screen[i + 1].position.x, vertices_screen[i + 2].position.x)) };
		const float minY{ std::min(vertices_screen[i].position.y, std::min(vertices_screen[i + 1].position.y, vertices_screen[i + 2].position.y)) };
		const float maxY{ std::max(vertices_screen[i].position.y, std::max(vertices_screen[i + 1].position.y, vertices_screen[i + 2].position.y)) };

		// If the triangle is outside the screen, skip it
		if (minX > m_Width - 1 || maxX < 0 || minY > m_Height - 1 || maxY < 0) continue;

		const int startX{ static_cast<int>(std::max(minX, .0f)) };
		const int endX{ static_cast<int>(std::min(maxX, static_cast<float>(m_Width - 1))) };
		const int startY{ static_cast<int>(std::max(minY, .0f)) };
		const int endY{ static_cast<int>(std::min(maxY, static_cast<float>(m_Height - 1))) };

		const float area
		{ Vector2::Cross(
			{ vertices_screen[i + 1].position.x - vertices_screen[i + 0].position.x, vertices_screen[i + 1].position.y - vertices_screen[i + 0].position.y },
			{ vertices_screen[i + 2].position.x - vertices_screen[i + 0].position.x, vertices_screen[i + 2].position.y - vertices_screen[i + 0].position.y })
		};

		for (int px{ startX }; px < endX; ++px)
		{
			for (int py{ startY }; py < endY; ++py)
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
		// Change your index loop accordingly(not pixel loop!). Considering if it’s an odd or even triangle if
		// using the triangle strip technique.Hint: odd or even ? -> modulo or bit masking

		for (size_t i{ 0 }; i < mesh.indices.size() - 2; ++i)
		{
			const Vertex& v0{ mesh.vertices[mesh.indices[i + 0]] };
			const Vertex& v1{ mesh.vertices[mesh.indices[i + 1]] };
			const Vertex& v2{ mesh.vertices[mesh.indices[i + 2]] };

			if (i + 2 > mesh.indices.size()) break;

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
	//Optimize vertices_out
	vertices_out.clear();
	vertices_out.reserve(vertices_in.size());

	const float aspectRatioFov{ static_cast<float>(m_Width) / static_cast<float>(m_Height) * m_Camera.fov };
	const float fovReciprocal{ 1.f / m_Camera.fov };

	for (const Vertex& vertex : vertices_in)
	{
		//transform vertex from world space to view space
		Vector3 viewSpacePos{ m_Camera.viewMatrix.TransformPoint(vertex.position) };

		const Vertex projectedVertex
		{
			{
				(viewSpacePos.x / viewSpacePos.z / aspectRatioFov + 1.f) * (m_Width * .5f),
				(1.f - viewSpacePos.y / viewSpacePos.z * fovReciprocal) * (m_Height * .5f),
				viewSpacePos.z
			},
			vertex.color
		};

		//add the vertex to vertices_out
		vertices_out.emplace_back(projectedVertex);
	}
}
void Renderer::VertexTransformationFunction(const std::vector<Mesh>& meshes_in, std::vector<Mesh>& meshes_out) const //W2 Version
{
	//Optimize vertices_out
	meshes_out.clear();
	meshes_out.reserve(meshes_in.size() * 3);

	// Copy meshes_in to meshes_out
	meshes_out = meshes_in;

	const float aspectRatioFov{ static_cast<float>(m_Width) / static_cast<float>(m_Height) * m_Camera.fov };
	const float fovReciprocal{ 1.f / m_Camera.fov };

	for (Mesh& mesh : meshes_out)
	{
		for (Vertex& vertex : mesh.vertices)
		{
			//Transform vertex from world space to view space
			Vector3 viewSpacePos{ m_Camera.viewMatrix.TransformPoint(vertex.position) };

			// Use correct perspective division
			vertex.position.x = (viewSpacePos.x / viewSpacePos.z / aspectRatioFov + 1.f) * (m_Width * .5f);
			vertex.position.y = (1.f - viewSpacePos.y / viewSpacePos.z * fovReciprocal) * (m_Height * .5f);
			vertex.position.z = viewSpacePos.z;
		}
	}
}

void dae::Renderer::RenderTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Texture* pTexture) const
{
	// Calculate the bounding box (Add small offset to prevent rounding errors)
	static constexpr float offset{ .5f };

	const int minX{ static_cast<int>(std::min(v0.position.x, std::min(v1.position.x, v2.position.x)) - offset) };
	const int maxX{ static_cast<int>(std::max(v0.position.x, std::max(v1.position.x, v2.position.x)) + offset) };
	const int minY{ static_cast<int>(std::min(v0.position.y, std::min(v1.position.y, v2.position.y)) - offset) };
	const int maxY{ static_cast<int>(std::max(v0.position.y, std::max(v1.position.y, v2.position.y)) + offset) };

	// If the triangle is outside the screen, skip it
	if (minX > m_Width - 1 || maxX < 0 || minY > m_Height - 1 || maxY < 0) return;

	const int startX{ std::max(minX, 0) };
	const int endX{ std::min(maxX, m_Width - 1) };
	const int startY{ std::max(minY, 0) };
	const int endY{ std::min(maxY, m_Height - 1) };

	const float area
	{ Vector2::Cross(
		{ v1.position.x - v0.position.x, v1.position.y - v0.position.y },
		{ v2.position.x - v0.position.x, v2.position.y - v0.position.y })
	};

	if (area == 0.f) return;

	// Loop over the bounding box
	for (int py{ startY }; py < endY; ++py)
	{
		for (int px{ startX }; px < endX; ++px)
		{
			// Check if the pixel is inside the triangle
			// If so, draw the pixel
			const Vector2 pixel{ static_cast<float>(px) + .5f, static_cast<float>(py) + .5f };

			float w0
			{ Vector2::Cross(
				{ pixel.x - v1.position.x, pixel.y - v1.position.y },
				{ pixel.x - v2.position.x, pixel.y - v2.position.y })
			};
			if (w0 < 0) continue;

			float w1
			{ Vector2::Cross(
				{ pixel.x - v2.position.x, pixel.y - v2.position.y },
				{ pixel.x - v0.position.x, pixel.y - v0.position.y })
			};
			if (w1 < 0) continue;

			float w2{ area - w0 - w1 };
			if (w2 < 0) continue;

			// Calculate the depth account for perspective interpolation
			const float z{ 1.f / ((1.f / v0.position.z * w0 + 1.f / v1.position.z * w1 + 1.f / v2.position.z * w2) / area) };

			//Check if pixel is in front of the current pixel in the depth buffer
			// Calculate buffer index
			const int bufferIdx{ px + (py * m_Width) };

			if (z > m_pDepthBufferPixels[bufferIdx])
				continue;

			//Update depth buffer
			m_pDepthBufferPixels[bufferIdx] = z;

			w0 /= area;
			w1 /= area;
			w2 /= area;

			ColorRGB finalColor{};

			if (pTexture)
			{
				// Interpolate uv coordinates correct for perspective
				const Vector2 uv{ (v0.uv / v0.position.z * w0 + v1.uv / v1.position.z * w1 + v2.uv / v2.position.z * w2) * z };

				// Sample the texture
				finalColor = pTexture->Sample(uv);
			}
			else
			{
				// Interpolate color correct for perspective
				finalColor = v0.color * w0 + v1.color * w1 + v2.color * w2;
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[bufferIdx] = SDL_MapRGB(m_pBackBuffer->format,
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