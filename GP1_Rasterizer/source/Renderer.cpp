//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"
#include <iostream>

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

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
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
	Render_W1_Part2(); //Projection Stage (Camera)

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::Render_W1_Part1()
{
	// Define Triangle - Vertices in NDC space
	static const std::vector<Vector3> vertices_ndc
	{
		{ .0f, .5f, 1.f },
		{ .5f, -.5f, 1.f },
		{ -.5f, -.5f, 1.f },
	};

	// Half screen size (for NDC to SCREEN space conversion)
	static const float halfWidth{ m_Width * .5f };
	static const float halfHeight{ m_Height * .5f };

	// Define Triangle - Vertices in SCREEN space
	static const std::vector<Vector3> vertices_screenSpace
	{
		{ (vertices_ndc[0].x + 1.f) * halfWidth, (1.f - vertices_ndc[0].y) * halfHeight, vertices_ndc[0].z },
		{ (vertices_ndc[1].x + 1.f) * halfWidth, (1.f - vertices_ndc[1].y) * halfHeight, vertices_ndc[1].z },
		{ (vertices_ndc[2].x + 1.f) * halfWidth, (1.f - vertices_ndc[2].y) * halfHeight, vertices_ndc[2].z }
	};

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			ColorRGB finalColor{};

			//Define Triangle Edges
			if (IsInsideTriangle({ static_cast<float>(px), static_cast<float>(py) }, vertices_screenSpace))
			{
				finalColor = colors::White;
			}

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
	// Define Triangle - Vertices in WORLD space
	static const std::vector<Vertex> vertices_world
	{
		{{ .0f, 2.f, .0f }},
		{{ 1.f, .0f, .0f }},
		{{ -1.f, .0f, .0f }},
	};

	static std::vector<Vertex> vertices_screen{};
	VertexTransformationFunction(vertices_world, vertices_screen);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			ColorRGB finalColor{};

			if (IsInsideTriangle({ static_cast<float>(px), static_cast<float>(py) }, vertices_screen))
			{
				finalColor = colors::White;
			}


			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	for (const Vertex& vertex : vertices_in)
	{
		//transform vertex from world space to view space
		Vector3 viewSpacePos{ m_Camera.viewMatrix.TransformPoint(vertex.position) };

		//perspective divide
		Vertex projectedVertex{};
		projectedVertex.position.x = viewSpacePos.x / viewSpacePos.z;
		projectedVertex.position.y = viewSpacePos.y / viewSpacePos.z;
		projectedVertex.position.z = viewSpacePos.z;

		//take camera settings into account
		projectedVertex.position.x /= (m_Width / static_cast<float>(m_Height) * m_Camera.fov);
		projectedVertex.position.y /= m_Camera.fov;

		//transform vertex to screen space
		projectedVertex.position.x = (projectedVertex.position.x + 1) / 2.f * m_Width;
		projectedVertex.position.y = (1 - projectedVertex.position.y) / 2.f * m_Height;

		//set color of the vertex
		projectedVertex.color = vertex.color;

		//add the vertex to vertices_out
		vertices_out.emplace_back(projectedVertex);
	}
}

bool dae::Renderer::IsInsideTriangle(const Vector2& pixel, const std::vector<Vector3>& vertices) const
{
	const Vector2 v0ToPixel{ pixel.x - vertices[0].x, pixel.y - vertices[0].y };
	const Vector2 v1ToPixel{ pixel.x - vertices[1].x, pixel.y - vertices[1].y };
	const Vector2 v2ToPixel{ pixel.x - vertices[2].x, pixel.y - vertices[2].y };

	const bool sign1{ Vector2::Cross(v0ToPixel, v1ToPixel) < 0.f };
	const bool sign2{ Vector2::Cross(v1ToPixel, v2ToPixel) < 0.f };
	const bool sign3{ Vector2::Cross(v2ToPixel, v0ToPixel) < 0.f };

	// Check if all signs are the same
	return (sign1 == sign2) && (sign2 == sign3);
}

bool dae::Renderer::IsInsideTriangle(const Vector2& pixel, const std::vector<Vertex>& vertices) const
{
	const Vector2 v0ToPixel{ pixel.x - vertices[0].position.x, pixel.y - vertices[0].position.y };
	const Vector2 v1ToPixel{ pixel.x - vertices[1].position.x, pixel.y - vertices[1].position.y };
	const Vector2 v2ToPixel{ pixel.x - vertices[2].position.x, pixel.y - vertices[2].position.y };

	const bool sign1{ Vector2::Cross(v0ToPixel, v1ToPixel) < 0.f };
	const bool sign2{ Vector2::Cross(v1ToPixel, v2ToPixel) < 0.f };
	const bool sign3{ Vector2::Cross(v2ToPixel, v0ToPixel) < 0.f };

	// Check if all signs are the same
	return (sign1 == sign2) && (sign2 == sign3);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}