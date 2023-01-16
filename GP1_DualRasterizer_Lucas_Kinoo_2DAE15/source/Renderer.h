#pragma once

#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class HardwareRasterizer;
	class SoftwareRasterizer;
	class Mesh;

	struct Camera;

	class Renderer final
	{
	public:
		explicit Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		// Function that returns wether or not the renderer is in hardware mode
		bool IsHardwareMode() const;

		bool ToggleFireFxMesh();
		bool ToggleMeshRotation() { m_RotateMesh = !m_RotateMesh; return m_RotateMesh; }
		bool ToggleSoftwareRasterizer() { m_RasterizerMode = m_RasterizerMode == RasterizerMode::Hardware ? RasterizerMode::Software : RasterizerMode::Hardware; return m_RasterizerMode == RasterizerMode::Software; }
		bool ToggleUniformClearColor() { m_UniformClearColor = !m_UniformClearColor; return m_UniformClearColor; }
		void CycleCullMode();
		void CycleTechniques() const;

	private:
		CullMode m_CullMode{ CullMode::Back };

		enum class RasterizerMode
		{
			Hardware,
			Software
		};
		RasterizerMode m_RasterizerMode{ RasterizerMode::Hardware };

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		const float m_RotationSpeed{ 45.f };

		bool m_RotateMesh{ true };
		bool m_UniformClearColor{ false };

		Camera* m_pCamera{ nullptr };
		std::vector<Mesh*> m_pMeshes{};

		const ColorRGB m_HardwareColor{ .39f, .59f, .93f };
		const ColorRGB m_SoftwareColor{ .39f, .39f, .39f };
		const ColorRGB m_UniformColor{ .1f, .1f, .1f };

		// Rasterizers
		HardwareRasterizer* m_pHardwareRasterizer{ nullptr };
		SoftwareRasterizer* m_pSoftwareRasterizer{ nullptr };

		void InitCamera();
		void InitVehicle(const Vector3& position = Vector3{ 0.f, 0.f, 0.f });

		void PrintKeybinds() const;
	};
}
