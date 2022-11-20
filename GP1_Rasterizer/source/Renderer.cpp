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
	m_pWindow(pWindow)
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

			float w2
			{ Vector2::Cross(
				{ pixel.x - vertices_screen[2].position.x, pixel.y - vertices_screen[2].position.y },
				{ pixel.x - vertices_screen[0].position.x, pixel.y - vertices_screen[0].position.y })
			};
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

				float w2
				{ Vector2::Cross(
					{ pixel.x - vertices_screen[i + 2].position.x, pixel.y - vertices_screen[i + 2].position.y },
					{ pixel.x - vertices_screen[i + 0].position.x, pixel.y - vertices_screen[i + 0].position.y })
				};
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

//void dae::Renderer::Render_W1_Part5()
//{
//	//Define Triangle - Vertices in WORLD space
//	static const std::vector<Vertex> vertices_world
//	{
//		//Triangle 0
//		{{ .0f, 2.f, .0f }, { 1.f, .0f, .0f }},
//		{{ 1.5f, -1.f, .0f }, { 1.f, .0f, .0f }},
//		{{ -1.5f, -1.f, .0f }, { 1.f, .0f, .0f }},
//
//		//Triangle 1
//		{{ .0f, 4.f, 2.f }, { 1.f, .0f, .0f }},
//		{{ 3.f, -2.f, 2.f }, { .0f, 1.f, .0f }},
//		{{ -3.f, -2.f, 2.f }, { .0f, .0f, 1.f }},
//	};
//
//	static std::vector<Vertex> vertices_screen;
//	VertexTransformationFunction(vertices_world, vertices_screen);
//
//	//RENDER LOGIC
//	for (int px{}; px < m_Width; ++px)
//	{
//		for (int py{}; py < m_Height; ++py)
//		{
//			ColorRGB finalColor{};
//
//			IsInsideTriangle({ static_cast<float>(px), static_cast<float>(py) }, vertices_screen, finalColor);
//
//			//Update Color in Buffer
//			finalColor.MaxToOne();
//
//			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
//				static_cast<uint8_t>(finalColor.r * 255),
//				static_cast<uint8_t>(finalColor.g * 255),
//				static_cast<uint8_t>(finalColor.b * 255));
//		}
//	}
//}

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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}