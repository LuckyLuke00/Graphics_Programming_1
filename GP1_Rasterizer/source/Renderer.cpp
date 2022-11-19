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
	//Render_W1_Part2(); //Projection Stage (Camera)
	Render_W1_Part3(); //Barycentric Coordinates

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::Render_W1_Part1()
{
	//Define Triangle - Vertices in NDC space
	static const std::vector<Vertex> vertices_ndc
	{
		{{ .0f, .5f, 1.f }},
		{{ .5f, -.5f, 1.f }},
		{{ -.5f, -.5f, 1.f }},
	};

	//Half screen size (for NDC to SCREEN space conversion)
	static const float halfWidth{ m_Width * .5f };
	static const float halfHeight{ m_Height * .5f };

	//Define Triangle - Vertices in SCREEN space
	static const std::vector<Vertex> vertices_screenSpace
	{
		{{ (vertices_ndc[0].position.x + 1.f) * halfWidth, (1.f - vertices_ndc[0].position.y) * halfHeight, vertices_ndc[0].position.z }},
		{{ (vertices_ndc[1].position.x + 1.f) * halfWidth, (1.f - vertices_ndc[1].position.y) * halfHeight, vertices_ndc[1].position.z }},
		{{ (vertices_ndc[2].position.x + 1.f) * halfWidth, (1.f - vertices_ndc[2].position.y) * halfHeight, vertices_ndc[2].position.z }},
	};

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			ColorRGB finalColor{};

			//Define Triangle Edges
			IsInsideTriangle({ static_cast<float>(px), static_cast<float>(py) }, vertices_screenSpace, finalColor);

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

			IsInsideTriangle({ static_cast<float>(px), static_cast<float>(py) }, vertices_screen, finalColor);

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
		{{ .0f, 4.f, 2.f }, { 1.f, .0f, .0f }},
		{{ 3.f, -2.f, 2.f }, { .0f, 1.f, .0f }},
		{{ -3.f, -2.f, 2.f }, { .0f, .0f, 1.f }},
	};

	static std::vector<Vertex> vertices_screen;
	VertexTransformationFunction(vertices_world, vertices_screen);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			ColorRGB finalColor{};

			IsInsideTriangle({ static_cast<float>(px), static_cast<float>(py) }, vertices_screen, finalColor);

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
	//Optimize vertices_out
	vertices_out.clear();
	vertices_out.reserve(vertices_in.size());

	for (const Vertex& vertex : vertices_in)
	{
		//transform vertex from world space to view space
		Vector3 viewSpacePos{ m_Camera.viewMatrix.TransformPoint(vertex.position) };

		//perspective divide
		const Vertex projectedVertex
		{
			{
				(viewSpacePos.x / viewSpacePos.z / (static_cast<float>(m_Width) / static_cast<float>(m_Height) * m_Camera.fov) + 1.f) * (m_Width * .5f),
				(1.f - viewSpacePos.y / viewSpacePos.z / m_Camera.fov) * (m_Height * .5f),
				viewSpacePos.z
			},
			vertex.color
		};

		//add the vertex to vertices_out
		vertices_out.emplace_back(projectedVertex);
	}
}

bool dae::Renderer::IsInsideTriangle(const Vector2& pixel, const std::vector<Vertex>& vertices, ColorRGB& pixelColor) const
{
	//Convert Vector3 to Vector2
	const Vector2 v0{ vertices[0].position.x, vertices[0].position.y };
	const Vector2 v1{ vertices[1].position.x, vertices[1].position.y };
	const Vector2 v2{ vertices[2].position.x, vertices[2].position.y };

	//Calculate the area of the total parallelogram
	const float totalArea{ Vector2::Cross(v1 - v0, v2 - v0) };

	//Calculate the weights
	const float weight0{ Vector2::Cross(v1 - pixel, v2 - pixel) / totalArea };
	if (weight0 < .0f) return false;

	const float weight1{ Vector2::Cross(v2 - pixel, v0 - pixel) / totalArea };
	if (weight1 < .0f) return false;

	const float weight2{ Vector2::Cross(v0 - pixel, v1 - pixel) / totalArea };
	if (weight2 < .0f) return false;

	pixelColor = vertices[0].color * weight0 + vertices[1].color * weight1 + vertices[2].color * weight2;
	return true;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}