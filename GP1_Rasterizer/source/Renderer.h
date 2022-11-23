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

		bool SaveBufferToImage() const;

		void Render();

		void Render_W3(); //Matrix Transformations

		void ToggleDepthBuffer();

		void Update(Timer* pTimer);

	private:
		SDL_Window* m_pWindow{ nullptr };

		SDL_Surface* m_pBackBuffer{ nullptr };
		SDL_Surface* m_pFrontBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{ nullptr };

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};

		// bool that toggles between finalColor and DepthBuffer
		bool m_RenderDepthBuffer{ false };

		// Texture
		const Texture* m_pTexture{ nullptr };

		void ClearBuffers(const Uint8& r = 100, const Uint8& g = 100, const Uint8& b = 100);

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const; //W3 Version

		void RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const Texture* pTexture = nullptr) const;
		void RenderMesh(const Mesh& mesh, const Texture* pTexture = nullptr) const;
	};
}
