#pragma once

#include "Camera.h"
#include "Texture.h"

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

		std::vector<Mesh*> GetMeshes() const { return m_pMeshes; }

	private:
		SDL_Window* m_pWindow{};

		const float m_RotationSpeed{ 45.f };

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };
		bool m_RotateMesh{ false };

		Camera* m_pCamera;
		Mesh* m_pMesh;
		std::vector<Mesh*> m_pMeshes;

		void InitCamera();
		void InitVehicle(const bool rotate = false);

		//DirectX
		HRESULT InitializeDirectX();

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
