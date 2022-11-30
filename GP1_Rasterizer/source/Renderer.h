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

		void ToggleDepthBuffer();
		void ToggleRotation();
		void ToggleNormalMap();
		void CycleShadingMode();

		void Update(const Timer* pTimer);

	private:
		SDL_Window* m_pWindow{ nullptr };

		SDL_Surface* m_pBackBuffer{ nullptr };
		SDL_Surface* m_pFrontBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{ nullptr };

		bool m_RenderDepthBuffer{ false };
		bool m_RenderNormalMap{ true };
		bool m_RotateMesh{ true };

		Camera m_Camera{};

		const float m_MeshRotationAngle{};

		float m_AspectRatio{};

		int m_Height{};
		int m_Width{};

		// Float of the width and height of the window
		float m_fHeight{};
		float m_fWidth{};

		float* m_pDepthBufferPixels{};

		const Texture* m_pGlossMap{ nullptr };
		const Texture* m_pNormalMap{ nullptr };
		const Texture* m_pSpecularMap{ nullptr };
		const Texture* m_pTexture{ nullptr };

		std::vector<Mesh> m_Meshes{};

		void ClearBuffers(const Uint8& r = 0, const Uint8& g = 0, const Uint8& b = 0) const;

		void RenderMesh(const Mesh& mesh, const Texture* pTexture = nullptr) const;
		void RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const Texture* pTexture = nullptr) const;

		static float EdgeFunction(const Vector2& a, const Vector2& b, const Vector2& c);

		// Pixel shading functions
		ColorRGB PixelShading(const Vertex_Out& v) const;

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const;
		void CalculateBoundingBox(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Int2& min, Int2& max) const;
		// Function that loops over the bounding box

		void InitializeMesh(const char* path, const Matrix& worldMatrix = {}, const PrimitiveTopology& topology = PrimitiveTopology::TriangleList);

		// Function that checks if a vertex is outside the view frustum
		bool IsOutsideViewFrustum(const Vertex_Out& v) const;

		enum class ShadingMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined,
		};

		ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };
	};
}
