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

		void SetCullMode(CullMode cullMode) { m_CullMode = cullMode; }
		void CycleShadingMode();
		void ToggleBoundingBox() { m_RenderBoundingBox = !m_RenderBoundingBox; }
		void ToggleDepthBuffer() { m_RenderDepthBuffer = !m_RenderDepthBuffer; }
		void ToggleNormalMap() { m_RenderNormalMap = !m_RenderNormalMap; }

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

		// Float of the width and height of the window
		float m_fHeight{};
		float m_fWidth{};

		float* m_pDepthBufferPixels{};

		CullMode m_CullMode{ CullMode::Back };

		const Texture* m_pGlossMap{ nullptr };
		const Texture* m_pNormalMap{ nullptr };
		const Texture* m_pSpecularMap{ nullptr };
		const Texture* m_pTexture{ nullptr };

		std::vector<Mesh*> m_pMeshes{};

		void ClearDepthBuffer() const;
		void ClearBackBuffer(const ColorRGB& clearColor) const;

		void RenderMesh(const Mesh* pMesh) const;
		void RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const;
		ColorRGB PixelShading(const Vertex_Out& v) const;

		static float EdgeFunction(const Vector2& a, const Vector2& b, const Vector2& c);

		bool IsOutsideViewFrustum(const Vertex_Out& v) const;
		void CalculateBoundingBox(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Int2& min, Int2& max) const;
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const;
	};
}
