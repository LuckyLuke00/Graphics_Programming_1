#pragma once

#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void ToggleMeshRotation() { m_RotateMesh = !m_RotateMesh; }
		void ToggleClearColor();
		void ToggleFireFXMesh();

		void CycleCullMode();

		std::vector<Mesh*> GetMeshes() const { return m_pMeshes; }

	private:
		SDL_Window* m_pWindow{};

		bool m_EnableUniformColor{ false };
		const ColorRGB m_HardwareColor{ .39f, .59f, .93f };
		const ColorRGB m_UniformColor{ .1f, .1f, .1f };
		ColorRGB m_ClearColor;

		const float m_RotationSpeed{ 45.f };

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };
		bool m_RotateMesh{ true };

		Camera* m_pCamera;
		std::vector<Mesh*> m_pMeshes;

		void InitCamera();
		void InitVehicle();

		//DirectX
		HRESULT InitializeDirectX();

		ID3D11RasterizerState* m_pRasterizerState{};
		D3D11_RASTERIZER_DESC  m_RasterizerDesc{};

		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};

		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};

		ID3D11Texture2D* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};
		//...
	};
}
