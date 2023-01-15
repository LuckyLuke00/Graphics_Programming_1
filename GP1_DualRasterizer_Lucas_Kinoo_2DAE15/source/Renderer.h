#pragma once

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

		bool ToggleMeshRotation() { m_RotateMesh = !m_RotateMesh; return m_RotateMesh; }
		void CycleCullMode();

	private:
		enum class CullMode
		{
			Back,
			Front,
			None,
		};
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

		bool m_IsInitialized{ false };
		bool m_RotateMesh{ true };

		Camera* m_pCamera{ nullptr };
		std::vector<Mesh*> m_pMeshes{};

		// Rasterizers
		HardwareRasterizer* m_pHardwareRasterizer{ nullptr };

		void InitCamera();
		void InitVehicle();

		void PrintKeybinds() const;
	};
}
