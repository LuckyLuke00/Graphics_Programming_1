#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;

	class HardwareRasterizer final
	{
	public:
		explicit HardwareRasterizer(SDL_Window* pWindow);
		~HardwareRasterizer();

		HardwareRasterizer(const HardwareRasterizer&) = delete;
		HardwareRasterizer(HardwareRasterizer&&) noexcept = delete;
		HardwareRasterizer& operator=(const HardwareRasterizer&) = delete;
		HardwareRasterizer& operator=(HardwareRasterizer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render(const std::vector<Mesh*>& pMeshes, const ColorRGB& clearColor) const;

		// Getters
		ID3D11Device* GetDevice() const { return m_pDevice; }
		ID3D11DeviceContext* GetDeviceContext() const { return m_pDeviceContext; }

		// Setters
		void SetCullMode(D3D11_CULL_MODE cullMode);

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		//DIRECTX
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
	};
}
