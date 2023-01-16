#pragma once
#include "DataTypes.h"

namespace dae
{
	class Mesh;
	class Texture;
	struct Vertex_Out;

	class SoftwareRasterizer final
	{
	public:
		explicit SoftwareRasterizer(SDL_Window* pWindow);
		~SoftwareRasterizer();

		SoftwareRasterizer(const SoftwareRasterizer&) = delete;
		SoftwareRasterizer(SoftwareRasterizer&&) noexcept = delete;
		SoftwareRasterizer& operator=(const SoftwareRasterizer&) = delete;
		SoftwareRasterizer& operator=(SoftwareRasterizer&&) noexcept = delete;

		void Render(const ColorRGB& clearColor);
		bool SaveBufferToImage() const;

		void SetMeshes(const std::vector<Mesh*>& meshes) { m_pMeshes = meshes; }
		void SetCullMode(CullMode cullMode) { m_CullMode = cullMode; }
		void CycleShadingMode();
		bool ToggleBoundingBox() { m_RenderBoundingBox = !m_RenderBoundingBox; return m_RenderBoundingBox; }
		bool ToggleDepthBuffer() { m_RenderDepthBuffer = !m_RenderDepthBuffer; return m_RenderDepthBuffer; }
		bool ToggleNormalMap() { m_RenderNormalMap = !m_RenderNormalMap; return m_RenderNormalMap; }

	private:
		enum class ShadingMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined,
		};
		ShadingMode m_ShadingMode{ ShadingMode::Combined };

		const LightingData m_LightingData{};

		SDL_Window* m_pWindow{ nullptr };

		SDL_Surface* m_pBackBuffer{ nullptr };
		SDL_Surface* m_pFrontBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{ nullptr };

		bool m_RenderBoundingBox{ false };
		bool m_RenderDepthBuffer{ false };
		bool m_RenderNormalMap{ true };

		float m_AspectRatio{};

		int m_Height{};
		int m_Width{};

		int m_CurrentMeshIndex{ 0 };

		// Float of the width and height of the window
		float m_fHeight{};
		float m_fWidth{};

		float* m_pDepthBufferPixels{};

		CullMode m_CullMode{ CullMode::Back };

		std::vector<Mesh*> m_pMeshes{};

		void ClearDepthBuffer() const;
		void ClearBackBuffer(const ColorRGB& clearColor) const;

		void RenderMesh(const Mesh* pMesh) const;
		void RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const;
		ColorRGB PixelShading(const Vertex_Out& v) const;

		static float EdgeFunction(const Vector2& a, const Vector2& b, const Vector2& c);

		bool IsOutsideViewFrustum(const Vertex_Out& v) const;
		void CalculateBoundingBox(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Int2& min, Int2& max) const;
		void VertexTransformationFunction(const std::vector<Mesh*>& pMeshes) const;
	};
}
