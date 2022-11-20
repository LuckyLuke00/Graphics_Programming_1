#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	//Forward Declarations
	class Scene;
	class Texture;
	class Timer;
	struct Mesh;
	struct Vertex;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		void Render_W1_Part1(); //Rasterizer Stage Only
		void Render_W1_Part2(); //Projection Stage (Camera)
		void Render_W1_Part3(); //Barycentric Coordinates
		void Render_W1_Part4(); //Depth Buffer
		void Render_W1_Part5(); //BoundingBox Optimization

		bool SaveBufferToImage() const;

	private:
		SDL_Window* m_pWindow{ nullptr };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{ nullptr };

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; //W1 Version

		//Triangle Intersection Test
		bool IsInsideTriangle(const Vector2& pixel, const std::vector<Vertex>& vertices, ColorRGB& pixelColor) const;
	};
}
