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
		void Render_Tuktuk();

		void ToggleDepthBuffer();

		void Update(const Timer* pTimer);

	private:
		SDL_Window* m_pWindow{ nullptr };

		SDL_Surface* m_pBackBuffer{ nullptr };
		SDL_Surface* m_pFrontBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{ nullptr };

		bool m_RenderDepthBuffer{ false };
		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		// Float of the width and height of the window
		float m_fWidth{};
		float m_fHeight{};

		float m_AspectRatio{};

		const Texture* m_pTexture{ nullptr };
		std::vector<Mesh> m_Meshes{};

		void ClearBuffers(const Uint8& r = 0, const Uint8& g = 0, const Uint8& b = 0);

		void RenderMesh(const Mesh& mesh, const Texture* pTexture = nullptr) const;
		void RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const Texture* pTexture = nullptr) const;

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const; //W3 Version

		void InitializeMesh(const std::string& path, const Matrix& worldMatrix = {}, const PrimitiveTopology& topology = PrimitiveTopology::TriangleList);

		// Function that checks if a vertex is outside the view frustum
		bool IsOutsideViewFrustum(const Vertex_Out& v) const;
	};
}
